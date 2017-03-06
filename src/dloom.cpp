//
// Copyright (C) 2017 Barcelona Supercomputing Center
//                    Centro Nacional de Supercomputacion
//
// This file is part of the Data Scheduler, a daemon for tracking and managing
// requests for asynchronous data transfer in a hierarchical storage environment.
//
// See AUTHORS file in the top level directory for information
// regarding developers and contributors.
//
// The Data Scheduler is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Data Scheduler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Data Scheduler.  If not, see <http://www.gnu.org/licenses/>.
//

#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstdlib>
#include <exception>
#include <dirent.h>
#include <fstream>
#include <fcntl.h>
#include <list>
#include <tbb/tbb.h>
#include <signal.h>
#include <ev.h>
#include <string>
#include <vector>
#include <ctime>

#include "ctpl.h" 

#define RUNNING_DIR "/tmp"  
#define SOCKET_FILE "/tmp/dloom.socket"  
#define DAEMON_LOCK_FILE "/tmp/dloom.lock"
#define LOG_FILE "/tmp/dloom.log"

extern "C" {
	struct foo {
		pid_t pid;
		uint64_t taskId;
		const char* filePath;
	};
}

struct task{
	pid_t pid;
	uint64_t taskId;
	const char *filePath;
};

struct task_finished{
	pid_t pid;
	uint64_t taskId;
	uint64_t status;
};

enum job_type{
	enqueue_task,
	check_task_status, 
	check_all_tasks
};

struct sock_ev_client;

struct sock_ev_serv {
	ev_io io;
	int fd;
	struct sockaddr_un socket;
	int socket_len;
	std::vector<sock_ev_client> clients;
};

struct sock_ev_client {
	ev_io io;
	int fd;
	int index;
	struct sock_ev_serv *server;
};


tbb::concurrent_queue<task> jobs_priority_1;
tbb::concurrent_hash_map<pid_t, std::list<task_finished>> tasks_finished_map;

void log_message(const char filename[], const char message[]){
	time_t now = time(0);
	char* dt = ctime(&now);
	FILE *logfile;
	logfile=fopen(filename, "a+");
	if(logfile == NULL){
		return;
	}
	if (0 > fprintf(logfile,"%s -- %s", message, dt)){
		perror("log_message, fprintf");
	}
	if( 0 != fclose(logfile)){
		perror("log_message, fclose");
	}
}

void signal_handler(int sig){
	switch(sig) {
		case SIGHUP: 
			log_message(LOG_FILE, "Hangup signal catched");
			break;
		case SIGTERM:
			log_message(LOG_FILE, "Terminate signal catched");
			exit(0);
			break;
		default:
			log_message(LOG_FILE, "Unknown signal received");
			break;
		}	
}

void daemonize() {
	/*
	 * --- Daemonize structure ---
	 *	Check if this is already a daemon
	 *  Fork off praent process
	 *	Obtain new process group
	 *	Close all descriptors
	 *	Handle standard IO
	 *  Change file mode mask
	 *	Change the current working directory
	 *  Check if daemon already exists
	 *	Manage signals
	 */

	log_message(LOG_FILE, "***************************");
	log_message(LOG_FILE, "** Starting dloom daemon **");
	log_message(LOG_FILE, "***************************");

	pid_t pid, sid;

	/* Check if this is already a daemon */ 
	if (getpid() == 1) return;
	
	/* Fork off the parent process */
	pid = fork();

	/* Fork error */
	if (pid < 0) {
		perror("Fork");
		exit(1);
	}

	/* Parent exits */
	if (pid > 0) {
		exit(0);
	}

	/* Obtain new process group */
	sid = setsid();
	if (sid < 0) {
		/* Log failure */
		throw std::exception();
	}

	/* Close all descriptors */
	int i;
	for(i=getdtablesize(); i>=0; --i){
		close(i);
	} 

	/* Handle standard IO */

	i=open("/dev/null", O_RDWR); /* open stdin */
	dup(i); /* stdout */
	dup(i); /* stderr */

	/* Change the file mode mask */
	umask(027); /* file creation mode to 750 */

	/* Change the current working directory */
	if ((chdir(RUNNING_DIR)) < 0) {
		perror("Chdir");
		exit(1);
	}
	
	/* Check if daemon already exists:
	 * First instance of the daemon will lock the file so that other
	 * instances understand that an instnace is already running. 
	 */

	int lfp;
	lfp=open(DAEMON_LOCK_FILE, O_RDWR|O_CREAT, 0640);
	if(lfp<0){
		perror("Can not open daemon lock file");
		exit(1);
	} 
	if(lockf(lfp, F_TLOCK, 0)<0){
		perror("Another instance of this daemon already running");
		exit(1);
	}

	/* record pid in lockfile */
	char str[10];
	sprintf(str, "%d\n", getpid());
	write(lfp, str, strlen(str));

	/* Manage signals */
	signal(SIGCHLD,SIG_IGN); /* ignore child */
	signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP,signal_handler); /* catch hangup signal */
	signal(SIGTERM,signal_handler); /* catch kill signal */

}

int set_non_block(int fd){
	/* Add O_NONBLOCK to the file descriptor */
	int flags;
	flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	return fcntl(fd, F_SETFL, flags);
}

int unix_socket_init(sockaddr_un* socket_un, char* sock_path){
	int fd;
	if (unlink(sock_path) < 0){
		perror("unlink socket");
		exit(EXIT_FAILURE);		 
	}

	/* Setup unix socket listener */
	fd = socket(AF_UNIX, SOCK_STREAM, 0);	
	if (fd == -1){
		perror("echo server socket");
		exit(EXIT_FAILURE);
	}

	/* Set it non-blocking */
	if (set_non_block(fd) == -1){
		perror("echo server socket nonbloc");
		exit(EXIT_FAILURE);
	}

	/* Set it as Unix socket */
	socket_un->sun_family = AF_UNIX;
	strcpy(socket_un->sun_path, sock_path);

	return fd;
}

int server_init(sock_ev_serv *server, char *sock_path, int max_queue){
	server->fd = unix_socket_init(&server->socket, sock_path);
	server->socket_len = sizeof(server->socket.sun_family) + strlen(server->socket.sun_path);
	if(bind(server->fd, (sockaddr*) &server->socket, server->socket_len) == -1){
		perror("echo server bind");
		exit(EXIT_FAILURE);
	}
	if(listen(server->fd, max_queue) == -1){
		perror("listen");
		exit(EXIT_FAILURE);
	}
	return 0;
}

static void not_blocked(EV_P_ ev_periodic *w, int revents){
	log_message(LOG_FILE, "Not blocked");
}

static void client_cb(EV_P_ ev_io *w, int revents){
	/* to-do: receive data */
	/* A client has become readable */
	std::string buff;
	struct sock_ev_client* client = (struct sock_ev_client*) w;
	int n = recv(client->fd, &buff, sizeof(buff), 0);
	if(n<=0){

	}

}

inline static sock_ev_client client_new(int fd){
	sock_ev_client client;
	client.fd = fd;
	set_non_block(client.fd);
	ev_io_init(&client.io, client_cb, client.fd, EV_READ);

	return client;
}

static void server_cb(EV_P_ ev_io *w, int revents){
	log_message(LOG_FILE, "Unix stream socket has become readable");
	int client_fd;
	sock_ev_client client; 

	/* ev_io is the first member, watcher 'w' has the addres of the start of the sock_ev_serv */
	sock_ev_serv *server = (struct sock_ev_serv*) w;

	while(1){
		client_fd = accept(server->fd, NULL, NULL);
		if( client_fd == -1){
			if( errno != EAGAIN && errno != EWOULDBLOCK ){
				log_message(LOG_FILE, "accept() failed");
				perror("accept()");
				exit(EXIT_FAILURE);
			}
			break;
		}
		log_message(LOG_FILE, "accepted a client");
		client = client_new(client_fd);
		client.server = server;
		server->clients.push_back(client);
		client.index = server->clients.size()-1;
		ev_io_start(EV_A_ &client.io);
	
	}
}

void communication_thread(int id){
	int max_queue = 128;
	sock_ev_serv server;
	ev_periodic every_few_seconds;
	EV_P = ev_default_loop(0);

	/* create unix socket in non-blocking fashion */
	server_init(&server, SOCKET_FILE, max_queue);

	/* Wake up periodically */
	ev_periodic_init(&every_few_seconds, not_blocked, 0, 5, 0);
	ev_periodic_start(EV_A_ &every_few_seconds);

	/* Get notified whenever the socket is ready to read */
	ev_io_init(&server.io, server_cb, server.fd, EV_READ);
	ev_io_start(EV_A_ &server.io);

	log_message(LOG_FILE, "unix-socket-echo starting...");
	ev_loop(EV_A_ 0);

	close(server.fd);
}

void push_jobs(ctpl::thread_pool &p){
	/*
	 * 1. If p.n_idle > 0 (probably sleep between checks)
	 * 2. Push job
	 */
	/*
	while(1){
		task t;
		if (p.n_idle() > 0 and jobs_priority_1.unsafe_size() > 0){
			bool pop_status = jobs_priority_1.try_pop(t);
			while (!pop_status){
				// some kind of sleep
				pop_status = jobs_priority_1.try_pop(t);
			}
			p.push(serve_job, t);
		} else {
			//When there are no more jobs or available threads, sleep
			sleep(4);
		}

	}*/

}

void infinite_loop() {
	/*
	 *	Create thread pool
	 *	Set number of threads
	 *	Start infinite loop
	 */

	ctpl::thread_pool p(3);
	p.push(communication_thread);
	push_jobs(p);
}

int main() {
	daemonize();
	infinite_loop();   
}
