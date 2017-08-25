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

ssize_t send_data(int conn, const void* data, size_t size){

	size_t bsent = 0;	// bytes sent
	size_t bleft = size;// bytes left to send
	ssize_t n = 0;

	// send() might not send all the bytes we ask it to,
	// because the kernel can decide not to send all the
	// data out in one chunk
	while(bsent < size) {
		n = write(conn, data + bsent, bleft);

		if(n == -1) {
			break;
		}

        bsent += n;
        bleft -= n;
	}

	return (n == -1 ? n : (ssize_t) bsent);
}

static int send_message(int conn, const void* msg, size_t msg_size) {

    // transform the message size into network order and send it 
    // before the actual data

    uint64_t prefix = htonll(msg_size);
    assert(sizeof(prefix) == NORNS_RPC_HEADER_LENGTH); 

	if(send_data(conn, &prefix, sizeof(prefix)) < 0) {
        return NORNS_ERPCSENDFAILED;
    }

	if(send_data(conn, msg, msg_size) < 0) {
        return NORNS_ERPCSENDFAILED;
    }

    return NORNS_SUCCESS;
}

/* Register and describe a batch job */
int norns_register_job(struct norns_cred* auth, struct norns_job* job) {

    (void) auth;

    if(job->jb_nhosts <= 0 || job->jb_nbackends <= 0) {
        return NORNS_EBADPARAMS;
    }

    // first of all, build the request body so that 
    // we can compute the final size
    Norns__Rpc__Request__Job jobmsg = NORNS__RPC__REQUEST__JOB__INIT;
    jobmsg.id = job->jb_jobid;
    jobmsg.n_hosts = job->jb_nhosts; // save number of repeated hosts
    jobmsg.hosts = malloc(jobmsg.n_hosts*sizeof(char*));

    if(jobmsg.hosts == NULL) {
        // errno set to ENOMEM
        return -1;
    }

    for(size_t i=0; i<job->jb_nhosts; ++i){
        size_t len = strlen(job->jb_hosts[i]);

        if(job->jb_hosts[i] == NULL){
            continue;
        }

        char* str = strndup(job->jb_hosts[i], len);

        if(str == NULL) {
            // errno set to ENOMEM
            return -1;
        }

        jobmsg.hosts[i] = str;
    }

    jobmsg.n_backends = job->jb_nbackends;
    jobmsg.backends = 
        malloc(job->jb_nbackends*sizeof(Norns__Rpc__Request__Job__Backend*));
    
    if(jobmsg.backends == NULL){
        // errno set to ENOMEM
        return -1;
    }

    for(size_t i=0; i<job->jb_nbackends; ++i) {
        fprintf(stdout, "%s\n", job->jb_backends[i]->b_mount);

        jobmsg.backends[i] = malloc(sizeof(Norns__Rpc__Request__Job__Backend));

        if(jobmsg.backends[i] == NULL) {
            // errno set to ENOMEM
            return -1;
        }

        norns__rpc__request__job__backend__init(jobmsg.backends[i]);
        jobmsg.backends[i]->type = job->jb_backends[i]->b_type;
        size_t len = strlen(job->jb_backends[i]->b_mount);
        jobmsg.backends[i]->mount = strndup(job->jb_backends[i]->b_mount, len);

        if(jobmsg.backends[i]->mount == NULL) {
            // errno set to ENOMEM
            return -1;
        }

        jobmsg.backends[i]->quota = job->jb_backends[i]->b_quota;
    }

    Norns__Rpc__Request req = NORNS__RPC__REQUEST__INIT;
    req.type = NORNS__RPC__REQUEST__TYPE__REGISTER_JOB;
    req.job = &jobmsg;

    // fill the buffer
    size_t req_len = norns__rpc__request__get_packed_size(&req);
    void* req_buf = malloc(req_len);

    if(req_buf == NULL) {
        return -1;
    }

    norns__rpc__request__pack(&req, req_buf);

    int conn = connect_to_daemon();

    if(conn == -1) {
        return NORNS_ECONNFAILED;
    }

	// connection established, send the message
	if(send_message(conn, req_buf, req_len) < 0) {
	    return NORNS_ERPCSENDFAILED;
    }

    close(conn);

    return NORNS_SUCCESS;
}

