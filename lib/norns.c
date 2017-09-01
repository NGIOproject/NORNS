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
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>


#include <norns.h>
#include <norns-rpc.h>
#include "messages.pb-c.h"

const char* SOCKET_FILE = "/tmp/urd.socket";

/* Global variables */
int sock;
char buff[10] = "hola";

/* Function declaration */

/* Specify init and finit function as constructor and destructor */

__attribute__((constructor)) static void __norns_init(void);
__attribute__((destructor)) static void __norns_finit(void); 


void __norns_init(){
}

static int connect_to_daemon(void){

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

void __norns_finit(void){
	/*
	 * 
	 */
}

int norns_transfer(struct norns_iotd* iotdp) {
    struct norns_iotd* iotd_copy = (struct norns_iotd*) 
        malloc(sizeof(*iotd_copy));

    if(iotd_copy == NULL){
        errno = ENOMEM;
        return -1;
    }

    memcpy(iotd_copy, iotdp, sizeof(*iotd_copy));

    // capture important process information
    iotd_copy->__pid = getpid();
    iotd_copy->__jobid = 0;

    int sfd = connect_to_daemon();

    if(sfd == -1){
        // errno should have been correctly set by connect_to_daemon()
        return -1;
    }
	
	// connection established, send the request descriptor to the daemon and 
	// wait for a response
	if (write(sfd, iotd_copy, sizeof(*iotd_copy)) < 0){
        perror("writing on stream socket");
        return -1;
    }

    struct norns_iotd response;

    size_t nbytes = 0;

    while(nbytes < sizeof(response)){
        ssize_t n = 0; 
        
        if((n = read(sfd, ((void*)&response)+n, sizeof(response))) == -1 ){
            return -1;
        }

        nbytes += n;
    }

    // copy data from response to user-provided structure
    memcpy(iotdp, &response, sizeof(response));

    free(iotd_copy);

    close(sfd);

    return 0;
}

ssize_t recv_data(int conn, void* data, size_t size) {

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


ssize_t send_data(int conn, const void* data, size_t size) {

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

    void* buffer = malloc(expected_size);

    if(buffer == NULL) {
        goto recv_error;
    }

    if(recv_data(conn, buffer, expected_size) < 0) {
        free(buffer);
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


static void job_message_free(Norns__Rpc__Request__Job* msg) {

    assert(msg != NULL);

    if(msg->hosts != NULL) {
        for(size_t i=0; i<msg->n_hosts; ++i) {
            if(msg->hosts[i] != NULL) {
                free(msg->hosts[i]);
            }
        }
        free(msg->hosts);
    }

    if(msg->backends != NULL) {

        for(size_t i=0; i<msg->n_backends; ++i) {
            if(msg->backends[i] != NULL) {
                if(msg->backends[i]->mount != NULL) {
                    free(msg->backends[i]->mount);
                }
                free(msg->backends[i]);
            }
        }
        free(msg->backends);
    }
    free(msg);
}

static int job_message_init(struct norns_job* job, Norns__Rpc__Request__Job** msg) {

    int rv;
    *msg = NULL;

    assert(job != NULL);

    Norns__Rpc__Request__Job* jobmsg = 
        (Norns__Rpc__Request__Job*) malloc(sizeof(*jobmsg));

    if(jobmsg == NULL) {
        rv = NORNS_ENOMEM;
        goto error_cleanup;
    }

    norns__rpc__request__job__init(jobmsg);

    // add hosts
    jobmsg->n_hosts = job->jb_nhosts; // save number of repeated hosts
    jobmsg->hosts = calloc(jobmsg->n_hosts, sizeof(char*));

    if(jobmsg->hosts == NULL) {
        rv = NORNS_ENOMEM;
        goto error_cleanup;
    }

    for(size_t i=0; i<job->jb_nhosts; ++i){
        size_t len = strlen(job->jb_hosts[i]);

        if(job->jb_hosts[i] == NULL){
            continue;
        }

        jobmsg->hosts[i] = strndup(job->jb_hosts[i], len);

        if(jobmsg->hosts[i] == NULL) {
            rv = NORNS_ENOMEM;
            goto error_cleanup;
        }
    }

    // add backends
    jobmsg->n_backends = job->jb_nbackends;
    jobmsg->backends = 
        calloc(job->jb_nbackends, sizeof(Norns__Rpc__Request__Job__Backend*));
    
    if(jobmsg->backends == NULL){
        rv = NORNS_ENOMEM;
        goto error_cleanup;
    }

    for(size_t i=0; i<job->jb_nbackends; ++i) {
        jobmsg->backends[i] = malloc(sizeof(Norns__Rpc__Request__Job__Backend));

        if(jobmsg->backends[i] == NULL) {
            rv = NORNS_ENOMEM;
            goto error_cleanup;
        }

        norns__rpc__request__job__backend__init(jobmsg->backends[i]);
        jobmsg->backends[i]->type = job->jb_backends[i]->b_type;
        size_t len = strlen(job->jb_backends[i]->b_mount);
        jobmsg->backends[i]->mount = strndup(job->jb_backends[i]->b_mount, len);

        if(jobmsg->backends[i]->mount == NULL) {
            rv = NORNS_ENOMEM;
            goto error_cleanup;
        }

        jobmsg->backends[i]->quota = job->jb_backends[i]->b_quota;
    }

    *msg = jobmsg;
    return NORNS_SUCCESS;

error_cleanup:
    if(jobmsg != NULL) {
        job_message_free(jobmsg);
    }
    return rv;
}

static int 
__send_job_request(Norns__Rpc__Request__Type type, struct norns_cred* auth, 
                   uint32_t jobid, struct norns_job* job) {

    (void) auth;

    int rv;
    void* req_buf, *resp_buf;
    size_t req_len, resp_len;
    int conn = -1;
    Norns__Rpc__Request__Job* jobmsg = NULL; 
    Norns__Rpc__Response* resp = NULL;
    req_buf = resp_buf = NULL;
    req_len = resp_len = 0;

    Norns__Rpc__Request req = NORNS__RPC__REQUEST__INIT;
    req.type = type;
    req.has_jobid = true;
    req.jobid = jobid;

    // this request needs a valid job descriptor
    if(job != NULL) {
        if(job->jb_nhosts <= 0 || job->jb_nbackends <= 0) {
            return NORNS_EBADPARAMS;
        }

        if((rv = job_message_init(job, &jobmsg)) != NORNS_SUCCESS) {
            goto cleanup;
        }

        assert(jobmsg != NULL);
        req.job = jobmsg;
    }

    // fill the buffer
    req_len = norns__rpc__request__get_packed_size(&req);
    req_buf = malloc(req_len);

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

    rv = resp->code;

cleanup:
    if(jobmsg != NULL) {
        job_message_free(jobmsg);
    }

    if(req_buf != NULL) {
        free(req_buf);
    }

    if(resp_buf) {
        free(resp_buf);
    }

    if(conn != -1) {
        close(conn);
    }

    if(resp != NULL) {
        norns__rpc__response__free_unpacked(resp, NULL);
    }

    return rv;
}

/* Register and describe a batch job */
int 
norns_register_job(struct norns_cred* auth, uint32_t jobid, 
                   struct norns_job* job) {

    return __send_job_request(NORNS__RPC__REQUEST__TYPE__REGISTER_JOB,
                              auth, jobid, job);

}

/* Update an existing batch job */
int 
norns_update_job(struct norns_cred* auth, uint32_t jobid, 
                 struct norns_job* job) {

    return __send_job_request(NORNS__RPC__REQUEST__TYPE__UPDATE_JOB,
                              auth, jobid, job);
}


/* Remove a batch job from the system */
int norns_unregister_job(struct norns_cred* auth, uint32_t jobid) {

    return __send_job_request(NORNS__RPC__REQUEST__TYPE__UNREGISTER_JOB,
                              auth, jobid, NULL);
}


/* Add a process to a registered batch job */
int 
norns_add_process(struct norns_cred* auth, uint32_t jobid, pid_t pid) {

    (void) auth;
    (void) jobid;
    (void) pid;

    fprintf(stdout, "unimplemented\n");

    return 0;
}


/* Remove a process from a registered batch job */
int 
norns_remove_process(struct norns_cred* auth, uint32_t jobid, pid_t pid) {

    (void) auth;
    (void) jobid;
    (void) pid;

    fprintf(stdout, "unimplemented\n");

    return 0;
}


