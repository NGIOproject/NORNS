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
#include <stdarg.h>

#include <norns.h>
#include <norns-rpc.h>

#include "defaults.h"
#include "messages.pb-c.h"
#include "requests.h"
#include "daemon-communication.h"
#include "xmalloc.h"

static int connect_to_daemon(void);
static int send_message(int conn, const msgbuffer_t* buffer);
static int recv_message(int conn, msgbuffer_t* buffer);
static ssize_t recv_data(int conn, void* data, size_t size);
static ssize_t send_data(int conn, const void* data, size_t size);
static void print_hex(void* buffer, size_t bytes);




static int
send_request(norns_rpc_type_t type, norns_response_t* resp, ...) {

    int res;
    msgbuffer_t req_buf = MSGBUFFER_INIT();
    msgbuffer_t resp_buf = MSGBUFFER_INIT();

    va_list ap;
    va_start(ap, resp);

    // fetch args and pack them into a buffer
    switch(type) {
        case NORNS_SUBMIT_IOTASK:
        {
            const struct norns_iotd* iotdp =
                va_arg(ap, const struct norns_iotd*);

            if((res = pack_to_buffer(type, &req_buf, iotdp)) 
                    != NORNS_SUCCESS) {
                return res;
            }

            break;
        }

        case NORNS_REGISTER_JOB:
        case NORNS_UPDATE_JOB:
        case NORNS_UNREGISTER_JOB:
        {
            const struct norns_cred* auth = 
                va_arg(ap, const struct norns_cred*);
            const uint32_t jobid =
                va_arg(ap, const uint32_t);
            const struct norns_job* job =
                va_arg(ap, const struct norns_job*);

            if((res = pack_to_buffer(type, &req_buf, auth, jobid, job)) 
                    != NORNS_SUCCESS) {
                return res;
            }

            break;
        }

        case NORNS_ADD_PROCESS:
        case NORNS_REMOVE_PROCESS:
        {
            const struct norns_cred* auth =
                va_arg(ap, const struct norns_cred*);
            const uint32_t jobid =
                va_arg(ap, const uint32_t);
            const uid_t uid =
                va_arg(ap, const uid_t);
            const gid_t gid =
                va_arg(ap, const gid_t);
            const pid_t pid =
                va_arg(ap, const pid_t);

            if((res = pack_to_buffer(type, &req_buf, auth, jobid, uid, gid, pid)) 
                    != NORNS_SUCCESS) {
                return res;
            }

            break;
        }

        case NORNS_PING:
        {
            if((res = pack_to_buffer(type, &req_buf)) != NORNS_SUCCESS) {
                return res;
            }

            break;
        }

        default:
            return NORNS_ESNAFU;

    }

    // connect to urd
    int conn = connect_to_daemon();

    if(conn == -1) {
        res = NORNS_ECONNFAILED;
        goto cleanup_on_error;
    }

	// connection established, send the message
	if(send_message(conn, &req_buf) < 0) {
	    res = NORNS_ERPCSENDFAILED;
	    goto cleanup_on_error;
    }

    // wait for the urd's response
    if(recv_message(conn, &resp_buf) < 0) {
        res = NORNS_ERPCRECVFAILED;
	    goto cleanup_on_error;
    }

    if((res = unpack_from_buffer(&resp_buf, resp)) != NORNS_SUCCESS) {
	    goto cleanup_on_error;
    }

    va_end(ap);

    close(conn);

    return res;

cleanup_on_error:
    va_end(ap);
    return res;
}

int 
send_transfer_request(struct norns_iotd* iotdp) {

    int res;
    norns_response_t resp;

    // XXX add missing checks
    if(iotdp->io_taskid != 0 || false
            
            ) {
        return NORNS_EBADARGS;
    }

    if((res = send_request(NORNS_SUBMIT_IOTASK, &resp, iotdp)) 
            != NORNS_SUCCESS) {
        return res;
    }

    if(resp.r_type != NORNS_SUBMIT_IOTASK) {
        return NORNS_ESNAFU;
    }

    iotdp->io_taskid = resp.r_taskid;

    return resp.r_status;

#if 0
    int rv;
    void* req_buf, *resp_buf;
    size_t req_len, resp_len;
    int conn = -1;

    Norns__Rpc__Request* req = NULL;
    Norns__Rpc__Response* resp = NULL;
    req_buf = resp_buf = NULL;
    req_len = resp_len = 0;

    // XXX add missing checks
    if(iotdp->io_taskid != 0 || false
            
            ) {
        return NORNS_EBADARGS;
    }

    msgbuffer_t xbuffer;

    if((rv = pack_to_buffer(NORNS_SUBMIT_IOTASK, &xbuffer, iotdp)) != NORNS_SUCCESS) {
        goto cleanup_on_error;
    }

#if 0
    if((req = build_request_msg(NORNS_SUBMIT_IOTASK, iotdp)) == NULL) {
        rv = NORNS_ENOMEM;
        goto cleanup_on_error;
    }

    // fill the buffer
    req_len = norns__rpc__request__get_packed_size(req);
    req_buf = xmalloc_nz(req_len);

    if(req_buf == NULL) {
        rv = NORNS_ENOMEM;
        goto cleanup_on_error;
    }

    norns__rpc__request__pack(req, req_buf);
#endif

    conn = connect_to_daemon();

    if(conn == -1) {
        rv = NORNS_ECONNFAILED;
        goto cleanup_on_error;
    }

	// connection established, send the message
	if(send_message(conn, req_buf, req_len) < 0) {
	    rv = NORNS_ERPCSENDFAILED;
	    goto cleanup_on_error;
    }

    // wait for the daemon's response
    if(recv_message(conn, &resp_buf, &resp_len) < 0) {
        rv = NORNS_ERPCRECVFAILED;
	    goto cleanup_on_error;
    }

    resp = norns__rpc__response__unpack(NULL, resp_len, resp_buf);

    if(resp == NULL) {
        rv = NORNS_ERPCRECVFAILED;
        goto cleanup_on_error;
    }

    rv = resp->status;

cleanup_on_error:
    if(req != NULL) {
        xfree(req);
    }

    return rv;
#endif
}

int 
send_job_request(norns_rpc_type_t type, struct norns_cred* auth, 
                 uint32_t jobid, struct norns_job* job) {

    int res;
    norns_response_t resp;

    if((res = send_request(type, &resp, auth, jobid, job)) 
            != NORNS_SUCCESS) {
        return res;
    }

    if(resp.r_type != type) {
        return NORNS_ESNAFU;
    }

    return resp.r_status;
}


int
send_process_request(norns_rpc_type_t type, struct norns_cred* auth, 
                     uint32_t jobid, uid_t uid, gid_t gid, pid_t pid) {

    int res;
    norns_response_t resp;

    if((res = send_request(type, &resp, auth, jobid, uid, gid, pid)) 
            != NORNS_SUCCESS) {
        return res;
    }

    if(resp.r_type != type) {
        return NORNS_ESNAFU;
    }

    return resp.r_status;
}

int
send_ping_request() {

    int res;
    norns_response_t resp;

    if((res = send_request(NORNS_PING, &resp)) != NORNS_SUCCESS) {
        return res;
    }

    if(resp.r_type != NORNS_PING) {
        return NORNS_ESNAFU;
    }

    return resp.r_status;

}

static int connect_to_daemon(void) {

	struct sockaddr_un server;

	int sfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sfd < 0) {
	    return -1;
	}

	server.sun_family = AF_UNIX;
	strncpy(server.sun_path, norns_api_sockfile, sizeof(server.sun_path));
	server.sun_path[sizeof(server.sun_path)-1] = '\0';

	if (connect(sfd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
	    return -1;
    }

    return sfd;
}

static int
send_message(int conn, const msgbuffer_t* buffer) {

    if(buffer == NULL || buffer->b_data == NULL || buffer->b_size == 0) {
        return -1;
    }

    const void* msg_data = buffer->b_data;
    size_t msg_length = buffer->b_size;


    // transform the message size into network order and send it 
    // before the actual data
    uint64_t prefix = htonll(msg_length);

    assert(sizeof(prefix) == NORNS_RPC_HEADER_LENGTH); 

	if(send_data(conn, &prefix, sizeof(prefix)) < 0) {
        return -1;
    }

	if(send_data(conn, msg_data, msg_length) < 0) {
        return -1;
    }

    return 0;
}

static int 
recv_message(int conn, msgbuffer_t* buffer) {

    // first of all read the message prefix and decode it 
    // so that we know how much data to receive
    uint64_t prefix = 0;

    if(recv_data(conn, &prefix, sizeof(prefix)) < 0) {
        goto recv_error;
    }

//    print_hex(&prefix, sizeof(prefix));

    size_t msg_size = ntohll(prefix);

    if(msg_size == 0) {
        goto recv_error;
    }

    void* msg_data = xmalloc_nz(msg_size);

    if(msg_data == NULL) {
        goto recv_error;
    }

    if(recv_data(conn, msg_data, msg_size) < 0) {
        xfree(msg_data);
        goto recv_error;
    }

//    print_hex(msg_data, msg_size);

    buffer->b_data = msg_data;
    buffer->b_size = msg_size;

    return 0;

recv_error:
    buffer->b_data = NULL;
    buffer->b_size = 0;

    return -1;
}

static ssize_t recv_data(int conn, void* data, size_t size) {

    size_t brecvd = 0; // bytes received
    size_t bleft = size; // bytes left to receive
    ssize_t n = 0;

	while(brecvd < size) {
		n = read(conn, data + brecvd, bleft);

//		fprintf(stdout, "read %zd\n", n);

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

