/* * Copyright (C) 2017 Barcelona Supercomputing Center
 *                    Centro Nacional de Supercomputacion
 *
 * This file is part of the Data Scheduler, a daemon for tracking and managing
 * requests for asynchronous data transfer in a hierarchical storage environment.
 *
 * See AUTHORS file in the top level directory for information
 * regarding developers and contributors.
 *
 * The Data Scheduler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Data Scheduler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Data Scheduler.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include <norns.h>
#include <norns-rpc.h>

#include "messages.pb-c.h"
#include "requests.h"
#include "daemon-communication.h"
#include "xmalloc.h"

const char* SOCKET_FILE = "/tmp/urd.socket"; ///TODO defaults.h

static int connect_to_daemon(void);
static int send_message(int conn, const void* msg, size_t msg_size);
static int recv_message(int conn, void** msg, size_t* msg_size);
static ssize_t recv_data(int conn, void* data, size_t size);
static ssize_t send_data(int conn, const void* data, size_t size);
static void print_hex(void* buffer, size_t bytes);

static int remap_request_type(norns_request_t type) {
    switch(type) {
        case NORNS_SUBMIT_IOTASK:
            return NORNS__RPC__REQUEST__TYPE__SUBMIT_IOTASK;
        case NORNS_REGISTER_JOB:
            return NORNS__RPC__REQUEST__TYPE__REGISTER_JOB;
        case NORNS_UPDATE_JOB:
            return NORNS__RPC__REQUEST__TYPE__UPDATE_JOB;
        case NORNS_UNREGISTER_JOB:
            return NORNS__RPC__REQUEST__TYPE__UNREGISTER_JOB;
        case NORNS_ADD_PROCESS:
            return NORNS__RPC__REQUEST__TYPE__ADD_PROCESS;
        case NORNS_REMOVE_PROCESS:
            return NORNS__RPC__REQUEST__TYPE__REMOVE_PROCESS;
        default:
            return -1;
    }
}

int 
send_transfer_request(struct norns_iotd* iotdp) {
    int rv;
    void* req_buf, *resp_buf;
    size_t req_len, resp_len;
    int conn = -1;

    Norns__Rpc__Request__Task* taskmsg = NULL; 
    Norns__Rpc__Request* req = NULL;
    Norns__Rpc__Response* resp = NULL;
    req_buf = resp_buf = NULL;
    req_len = resp_len = 0;

    if(iotdp->io_taskid != 0 || false
            
            ) {
        return NORNS_EBADARGS;
    }


    if((req = (Norns__Rpc__Request*) xmalloc(sizeof(*req))) == NULL) {
        rv = NORNS_ENOMEM;
        goto cleanup;
    }

    norns__rpc__request__init(req);

    req->type = NORNS__RPC__REQUEST__TYPE__SUBMIT_IOTASK;

    // this request needs a valid task descriptor
    if(iotdp != NULL) {
        if((rv = build_task_message(iotdp, &taskmsg)) != NORNS_SUCCESS) {
            goto cleanup;
        }

        assert(taskmsg != NULL);
        req->task = taskmsg;
    }

    // fill the buffer
    req_len = norns__rpc__request__get_packed_size(req);
    req_buf = xmalloc_nz(req_len);

    if(req_buf == NULL) {
        rv = NORNS_ENOMEM;
        goto cleanup;
    }

    norns__rpc__request__pack(req, req_buf);

    conn = connect_to_daemon();

    if(conn == -1) {
        rv = NORNS_ECONNFAILED;
        goto cleanup;
    }

	// connection established, send the message
	if(send_message(conn, req_buf, req_len) < 0) {
	    rv = NORNS_ERPCSENDFAILED;
	    goto cleanup;
    }

    // wait for the daemon's response
    if(recv_message(conn, &resp_buf, &resp_len) < 0) {
        rv = NORNS_ERPCRECVFAILED;
	    goto cleanup;
    }

    resp = norns__rpc__response__unpack(NULL, resp_len, resp_buf);

    if(resp == NULL) {
        rv = NORNS_ERPCRECVFAILED;
        goto cleanup;
    }

    rv = resp->status;

cleanup:
    if(req != NULL) {
        xfree(req);
    }

    return rv;
}

int 
send_job_request(norns_request_t type, struct norns_cred* auth, 
                 uint32_t jobid, struct norns_job* job) {

    //TODO: encapsulate the construction of the request buffer into requests.c

    (void) auth;

    int rv;
    void* req_buf, *resp_buf;
    size_t req_len, resp_len;
    int conn = -1;

    Norns__Rpc__Request__Type req_type;
    Norns__Rpc__Request__Job* jobmsg = NULL; 
    Norns__Rpc__Response* resp = NULL;
    req_buf = resp_buf = NULL;
    req_len = resp_len = 0;

    if((req_type = remap_request_type(type)) < 0) {
        return NORNS_ESNAFU;
    }

    Norns__Rpc__Request req = NORNS__RPC__REQUEST__INIT;
    req.type = req_type;
    req.has_jobid = true;
    req.jobid = jobid;

    // this request needs a valid job descriptor
    if(job != NULL) {
        if(job->jb_nhosts <= 0 || job->jb_nbackends <= 0) {
            return NORNS_EBADARGS;
        }

        if((rv = job_message_init(job, &jobmsg)) != NORNS_SUCCESS) {
            goto cleanup;
        }

        assert(jobmsg != NULL);
        req.job = jobmsg;
    }

    // fill the buffer
    req_len = norns__rpc__request__get_packed_size(&req);
    req_buf = xmalloc_nz(req_len);

    if(req_buf == NULL) {
        rv = NORNS_ENOMEM;
        goto cleanup;
    }

    norns__rpc__request__pack(&req, req_buf);

    conn = connect_to_daemon();

    if(conn == -1) {
        rv = NORNS_ECONNFAILED;
        goto cleanup;
    }

	// connection established, send the message
	if(send_message(conn, req_buf, req_len) < 0) {
	    rv = NORNS_ERPCSENDFAILED;
	    goto cleanup;
    }

    // wait for the daemon's response
    if(recv_message(conn, &resp_buf, &resp_len) < 0) {
        rv = NORNS_ERPCRECVFAILED;
	    goto cleanup;
    }

    resp = norns__rpc__response__unpack(NULL, resp_len, resp_buf);

    if(resp == NULL) {
        rv = NORNS_ERPCRECVFAILED;
        goto cleanup;
    }

    rv = resp->status;

cleanup:
    if(jobmsg != NULL) {
        job_message_free(jobmsg);
    }

    if(req_buf != NULL) {
        xfree(req_buf);
    }

    if(resp_buf) {
        xfree(resp_buf);
    }

    if(conn != -1) {
        close(conn);
    }

    if(resp != NULL) {
        norns__rpc__response__free_unpacked(resp, NULL);
    }

    return rv;
}


int
send_process_request(norns_request_t type, struct norns_cred* auth, 
                     uint32_t jobid, uid_t uid, gid_t gid, pid_t pid) {

    (void) auth;

    int rv;
    void* req_buf, *resp_buf;
    size_t req_len, resp_len;
    int conn = -1;
    Norns__Rpc__Request__Type req_type;
    Norns__Rpc__Response* resp = NULL;
    req_buf = resp_buf = NULL;
    req_len = resp_len = 0;

    if((req_type = remap_request_type(type)) < 0) {
        return NORNS_ESNAFU;
    }

    Norns__Rpc__Request req = NORNS__RPC__REQUEST__INIT;
    req.type = req_type;
    req.has_jobid = true;
    req.jobid = jobid;

    Norns__Rpc__Request__Process proc = NORNS__RPC__REQUEST__PROCESS__INIT;
    proc.uid = uid;
    proc.gid = gid;
    proc.pid = pid;

    req.process = &proc;

    // fill the buffer
    req_len = norns__rpc__request__get_packed_size(&req);
    req_buf = xmalloc_nz(req_len);

    if(req_buf == NULL) {
        rv = NORNS_ENOMEM;
        goto cleanup;
    }

    norns__rpc__request__pack(&req, req_buf);

    conn = connect_to_daemon();

    if(conn == -1) {
        rv = NORNS_ECONNFAILED;
        goto cleanup;
    }

	// connection established, send the message
	if(send_message(conn, req_buf, req_len) < 0) {
	    rv = NORNS_ERPCSENDFAILED;
	    goto cleanup;
    }

    // wait for the daemon's response
    if(recv_message(conn, &resp_buf, &resp_len) < 0) {
        rv = NORNS_ERPCRECVFAILED;
	    goto cleanup;
    }

    resp = norns__rpc__response__unpack(NULL, resp_len, resp_buf);

    if(resp == NULL) {
        rv = NORNS_ERPCRECVFAILED;
        goto cleanup;
    }

    rv = resp->status;

cleanup:
    if(req_buf != NULL) {
        xfree(req_buf);
    }

    if(resp_buf) {
        xfree(resp_buf);
    }

    if(conn != -1) {
        close(conn);
    }

    if(resp != NULL) {
        norns__rpc__response__free_unpacked(resp, NULL);
    }

    return rv;
}

static int connect_to_daemon(void) {

	struct sockaddr_un server;

	int sfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sfd < 0) {
	    return -1;
	}

	server.sun_family = AF_UNIX;
	strncpy(server.sun_path, SOCKET_FILE, sizeof(server.sun_path));
	server.sun_path[sizeof(server.sun_path)-1] = '\0';

	if (connect(sfd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
	    return -1;
    }

    return sfd;
}

static int send_message(int conn, const void* msg, size_t msg_size) {

    // transform the message size into network order and send it 
    // before the actual data

    uint64_t prefix = htonll(msg_size);

    assert(sizeof(prefix) == NORNS_RPC_HEADER_LENGTH); 

	if(send_data(conn, &prefix, sizeof(prefix)) < 0) {
        return -1;
    }

	if(send_data(conn, msg, msg_size) < 0) {
        return -1;
    }

    return 0;
}

static int recv_message(int conn, void** msg, size_t* msg_size) {

    // first of all read the message prefix and decode it 
    // so that we know how much data to receive
    uint64_t prefix = 0;

    if(recv_data(conn, &prefix, sizeof(prefix)) < 0) {
        goto recv_error;
    }

    print_hex(&prefix, sizeof(prefix));

    size_t expected_size = ntohll(prefix);

    if(expected_size == 0) {
        goto recv_error;
    }

    void* buffer = xmalloc_nz(expected_size);

    if(buffer == NULL) {
        goto recv_error;
    }

    if(recv_data(conn, buffer, expected_size) < 0) {
        xfree(buffer);
        goto recv_error;
    }

    print_hex(buffer, expected_size);

    *msg = buffer;
    *msg_size = expected_size;

    return 0;

recv_error:
    *msg = NULL;
    *msg_size = 0;

    return -1;
}

static ssize_t recv_data(int conn, void* data, size_t size) {

    size_t brecvd = 0; // bytes received
    size_t bleft = size; // bytes left to receive
    ssize_t n = 0;

	while(brecvd < size) {
		n = read(conn, data + brecvd, bleft);

		fprintf(stdout, "read %zd\n", n);

		if(n == -1 || n == 0) {
		    if(errno == EINTR) {
		        continue;
            }
			break;
		}

        brecvd += n;
        bleft -= n;
	}

	return (n == -1 ? n : (ssize_t) brecvd);
}


static ssize_t send_data(int conn, const void* data, size_t size) {

	size_t bsent = 0;	// bytes sent
	size_t bleft = size;// bytes left to send
	ssize_t n = 0;

	// send() might not send all the bytes we ask it to,
	// because the kernel can decide not to send all the
	// data out in one chunk
	while(bsent < size) {
		n = write(conn, data + bsent, bleft);

		if(n == -1) {
		    if(errno == EINTR) {
		        continue;
            }
			break;
		}

        bsent += n;
        bleft -= n;
	}

	return (n == -1 ? n : (ssize_t) bsent);
}

static void print_hex(void* buffer, size_t bytes) {

    unsigned char* p = (unsigned char*) buffer;

    fprintf(stdout, "<< ");

    for(size_t i = 0; i < bytes; ++i) {
        fprintf(stdout, "%02x ", (int) p[i]);
    }

    fprintf(stdout, " >>\n");
}

