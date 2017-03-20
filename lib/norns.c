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

#include <norns.h>

const char* SOCKET_FILE = "/tmp/urd.socket";

/* Global variables */
int sock;
char buff[10] = "hola";

/* Function declaration */

/* Specify init and finit function as constructor and destructor */

__attribute__((constructor)) static void __norns_init(void);
__attribute__((destructor)) static void __norns_finit(void); 


void __norns_init(){

#if 0
	struct sockaddr_un server;

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("opening stream socket");
		exit(EXIT_FAILURE);
	}

	server.sun_family = AF_UNIX;
	strncpy(server.sun_path, SOCKET_FILE, sizeof(server.sun_path));
	server.sun_path[sizeof(server.sun_path)-1] = '\0';

	if (connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
        if (close(sock) < 0){
        	exit(EXIT_FAILURE);
        }
        perror("connecting stream socket");
        exit(EXIT_FAILURE);
    }
#endif

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
	printf("Executing this when the library is unloaded\n");
	close(sock);
}

int norns_transfer(struct norns_iotd* iotdp) {
    struct norns_iotd* iotd_copy = (struct norns_iotd*) 
        malloc(sizeof(*iotd_copy));

    if(iotd_copy == NULL){
        errno = ENOMEM;
        return -1;
    }

    memcpy(iotd_copy, iotdp, sizeof(*iotd_copy));

    int sfd = connect_to_daemon();

    if(sfd == -1){
        // errno should have been correctly set by connect_to_daemon()
        return -1;
    }
	
	// connection established, send the request descriptor to the daemon and 
	// wait for a response
	if (write(sfd, iotd_copy, sizeof(*iotd_copy)) < 0){
        perror("writing on stream socket");
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
