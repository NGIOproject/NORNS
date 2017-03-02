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

#include <norns.h>

#define SOCKET_NAME "urd_socket" 

struct task{
	pid_t pid;
	uint64_t taskId;
	const char *filePath;
};

void norns_init(){
	int sock;
	struct sockaddr_un server;
	char buff[1024] = "hola";
	struct task t;
	t.pid = 3;
	t.taskId = 4;

	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("opening stream socket");
		exit(1);
	}

	server.sun_family = AF_UNIX;
	strcpy(server.sun_path, SOCKET_NAME);

	if (connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
        if (close(sock) < 0)
        	exit(1);
        perror("connecting stream socket");
        exit(1);
    }
    if (write(sock, &t, sizeof(t)) < 0)
        perror("writing on stream socket");
    close(sock);
}

/* Enqueue an asynchronous I/O task */
int norns_transfer(struct norns_iotd *iotdp) {
    (void) iotdp;
    return 0;
}

/* Try to cancel an asynchronous I/O task associated with iotdp */
ssize_t norns_cancel(struct norns_iotd *iotdp) { 
    (void) iotdp;
    return 0;
}

/* Retrieve return status associated with iotdp */
ssize_t norns_return(struct norns_iotd *iotdp) {
    (void) iotdp;
    return 0;
}

/* Retrieve current status associated with iotdp */
ssize_t norns_progress(struct norns_iotd *iotdp) {
    (void) iotdp;
    return 0;
}

/* Retrieve error status associated with iotdp */
int norns_error(struct norns_iotd *iotdp) {
    (void) iotdp;
    return 0;
}


