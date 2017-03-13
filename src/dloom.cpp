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
#include <string.h>
#include <vector>
#include <ctime>

#include "ctpl.h" 

const char* RUNNING_DIR = "/tmp";
const char* SOCKET_FILE = "/tmp/dloom.socket";  
const char* DAEMON_LOCK_FILE = "/tmp/dloom.lock";
const char* LOG_FILE = "/tmp/dloom.log";
const int WAKEUP_PERIODIC_TIME = 5;
const int N_THREADS_IN_POOL = 3;
const int MAX_CLIENTS_SUPPORTED = 20;

ev_periodic every_few_seconds;

extern "C" {
	struct norns_iotd {
	    int ni_tid;           /* task identifier */
	    int ni_ibid;          /* source backend identifier */
	    const char* ni_ipath; /* path to data source */
	    int ni_obid;          /* destination backend identifier */
	    const char* ni_opath; /* path to data destination */
	    int ni_type;          /* operation to be performed */
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
	ENQUEUE_TASK,
	CHECK_TASK_STATUS,
	CHECK_ALL_TASKS
};

struct sock_ev_client;

struct sock_ev_serv {
	ev_io io;
	int fd;
	struct sockaddr_un socket;
	int socket_len;
	int max_clients;
	int current_clients;
};

struct sock_ev_client {
	ev_io io;
	int fd;
	int index;
	struct sock_ev_serv *server;
};

sock_ev_serv server;
tbb::concurrent_queue<task> jobs_priority_1;
tbb::concurrent_hash_map<pid_t, std::list<task_finished>> tasks_finished_map;

void log_message(const char filename[], const char message[]){
	/*
	 * In a future log messages will not be redirected to a file but to the syslog.
	 * We can use fprintf, systemd will redirect to syslog automatically.  
	 */
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
			log_message(LOG_FILE, "Shutting down dloom");
			exit(EXIT_SUCCESS);
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
	if (getpid() == 1){
		return;
	}
	
	/* Fork off the parent process */
	pid = fork();

	/* Fork error */
	if (pid < 0) {
		log_message(LOG_FILE, "[daemonize] fork failed.");
		perror("Fork");
		exit(EXIT_FAILURE);
	}

	/* Parent exits */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* Obtain new process group */
	sid = setsid();
	if (sid < 0) {
		/* Log failure */
		log_message(LOG_FILE, "[daemonize] setsid failed.");
		perror("Setsid");
		exit(EXIT_FAILURE);
	}

	/* Close all descriptors */
	int i;
	for(i=getdtablesize(); i>=0; --i){
		close(i);
	} 

	/* Handle standard IO */

	i=open("/dev/null", O_RDWR); /* open stdin */
	if(-1 == dup(i)){ /* stdout */
		log_message(LOG_FILE, "[daemonize] dup[1] failed.");
		perror("dup");
		exit(EXIT_FAILURE);
	}
	if(-1 == dup(i)){ /* stderr */
		log_message(LOG_FILE, "[daemonize] dup[2] failed.");
		perror("dup");
		exit(EXIT_FAILURE);
	}

	/* Change the file mode mask */
	umask(027); /* file creation mode to 750 */

	/* Change the current working directory */
	if ((chdir(RUNNING_DIR)) < 0) {
		log_message(LOG_FILE, "[daemonize] chdir failed.");
		perror("Chdir");
		exit(EXIT_FAILURE);
	}
	
	/* Check if daemon already exists:
	 * First instance of the daemon will lock the file so that other
	 * instances understand that an instnace is already running. 
	 */

	int lfp;
	lfp=open(DAEMON_LOCK_FILE, O_RDWR|O_CREAT, 0640);
	if(lfp<0){
		log_message(LOG_FILE, "[daemonize] can not open daemon lock file");
		perror("Can not open daemon lock file");
		exit(EXIT_FAILURE);
	} 
	if(lockf(lfp, F_TLOCK, 0)<0){
		log_message(LOG_FILE, "[daemonize] another instance of this daemon already running");
		perror("Another instance of this daemon already running");
		exit(EXIT_FAILURE);
	}

	/* record pid in lockfile */
	char str[10];
	size_t err_snprintf;
	err_snprintf = snprintf(str, sizeof(str), "%d\n", getpid());
	if(err_snprintf >= sizeof(str)){
		log_message(LOG_FILE, "[daemonize] snprintf failed");
	}
	size_t err_write;
	err_write = write(lfp, str, strnlen(str, sizeof(str)));
	if(err_write != strnlen(str, sizeof(str))){
		log_message(LOG_FILE, "[daemonize] write failed");
	}

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

int unix_socket_init(sockaddr_un* socket_un, const char* sock_path){
	int fd;

    struct stat stbuf;

    /* if socket exists from a previous run, remove it */
    if (stat(sock_path, &stbuf) == 0) {
        if (unlink(sock_path) < 0){
            log_message(LOG_FILE, "unlink socket failed");
            perror("unlink socket");
            exit(EXIT_FAILURE);		 
        }
    }

	/* Setup unix socket listener */
	fd = socket(AF_UNIX, SOCK_STREAM, 0);	
	if (fd == -1){
		log_message(LOG_FILE, "echo server socket failed");
		perror("echo server socket");
		exit(EXIT_FAILURE);
	}

	/* Set it non-blocking */
	if (set_non_block(fd) == -1){
		log_message(LOG_FILE, "echo server socket nonbloc failed");
		perror("echo server socket nonbloc");
		exit(EXIT_FAILURE);
	}

	/* Set it as Unix socket */
	socket_un->sun_family = AF_UNIX;
	strncpy(socket_un->sun_path, sock_path, sizeof(socket_un->sun_path));

	return fd;
}


int server_init(sock_ev_serv *serv, const char *sock_path){
	log_message(LOG_FILE, "initializing server...");
	serv->max_clients = MAX_CLIENTS_SUPPORTED;
	serv->fd = unix_socket_init(&serv->socket, sock_path);
	serv->socket_len = sizeof(serv->socket.sun_family) + strnlen(serv->socket.sun_path, sizeof(serv->socket.sun_path));
	if(bind(serv->fd, (sockaddr*) &serv->socket, serv->socket_len) == -1){
		perror("echo server bind");
		exit(EXIT_FAILURE);
	}
	if(listen(serv->fd, serv->max_clients) < -1){
		perror("listen");
		exit(EXIT_FAILURE);
	}
	return 0;
}


static void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents){
	/* to-do: receive data */
	/* A client has become readable */
	log_message(LOG_FILE, "Inside read_cb");
	(void)revents;
	(void)loop;

	norns_iotd t;
	ssize_t read;

	if(EV_ERROR & revents)
    {
    	log_message(LOG_FILE, "got invalid event in read_cb");
      	perror("got invalid event");
      	return;
    }

    //receive message from client socket
    read = recv(watcher->fd, &t, sizeof(t), 0);

    if(read < 0){
    	log_message(LOG_FILE, "read error");
    }

    if(read == 0){
    	//stop and free watcher if client socket is closing
    	ev_io_stop(loop, watcher);
    	free(watcher);
    	log_message(LOG_FILE, "client close socket");
    	server.current_clients = server.current_clients -1;
    	return;
    } else {
    	log_message(LOG_FILE, "we have received this message: ");
    	log_message(LOG_FILE, (std::to_string(t.ni_tid)).c_str());
    }
    
}


inline static sock_ev_client client_new(int fd){
	sock_ev_client client;
	client.fd = fd;
	set_non_block(client.fd);
	//ev_io_init(&client.io, client_cb, client.fd, EV_READ);
	log_message(LOG_FILE, "client_new registered");
	return client;
}

static void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents){
	/*
	 * Callback for accepting clients.
	 */
	(void)watcher;
	(void)revents;
	log_message(LOG_FILE, "Unix stream socket has become readable");

	struct sockaddr_un client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_sd;
	struct ev_io *w_client = (struct ev_io*) malloc (sizeof(struct ev_io));

	if(EV_ERROR & revents){
		log_message(LOG_FILE, "got invalide event");
		return;
	}

	/* Accept client request */
	client_sd = accept(server.fd, (struct sockaddr *) &client_addr, &client_len);

	if(client_sd < 0){
		log_message(LOG_FILE, "accept error");
		return;
	}

	/* Increment number of clients connected */
	server.current_clients = server.current_clients + 1;

	log_message(LOG_FILE, "succesfully connected with client.");

	/* Initialize and start watcher to read client requests */

	ev_io_init(w_client, read_cb, client_sd, EV_READ);
	ev_io_start(loop, w_client);
}

void communication_thread(int id){
	(void)id;
	
	/* loop is set to default. If different config is needed we need to change this. */
	struct ev_loop *loop = ev_default_loop(0);

	/* init server and socket */
	server_init(&server, SOCKET_FILE);

	/* Initialize and start a watcher to accept client requests */
	ev_io_init(&server.io, accept_cb, server.fd, EV_READ);
	ev_io_start(loop, &server.io);

	log_message(LOG_FILE, "unix-socket-echo starting...");
	
	while(1){
		ev_loop(EV_A_ 0);
	}
	
	/* Only reached if the loop is manually exited */
	log_message(LOG_FILE, "loop manually exited");
	close(server.fd);
	exit(EXIT_SUCCESS);
}

void push_jobs(ctpl::thread_pool &p){
	/*
	 * 1. If p.n_idle > 0 (probably sleep between checks)
	 * 2. Push job
	 */
	(void) p;
	while(1){
		//log_message(LOG_FILE, "[main_thread] not blocked");
		sleep(5);
	};
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

	ctpl::thread_pool p(N_THREADS_IN_POOL);
	p.push(communication_thread);
	push_jobs(p);
}

int main() {
	daemonize();
	infinite_loop();   
}
