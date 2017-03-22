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
#include <string.h>
#include <vector>
#include <ctime>

#include <boost/asio.hpp>
#include <functional>



#include <norns.h>

#include "ipc-listener.hpp"
#include "ctpl.h" 
#include "logger.hpp"
#include "urd.hpp"

#if 0 
const char* RUNNING_DIR = "/tmp";
const char* SOCKET_FILE = "/tmp/urd.socket";  
const char* DAEMON_LOCK_FILE = "/tmp/urd.lock";
const char* LOG_FILE = "/tmp/urd.log";
const int WAKEUP_PERIODIC_TIME = 5;
const int N_THREADS_IN_POOL = 3;
const int MAX_CLIENTS_SUPPORTED = 20;

ev_periodic every_few_seconds;

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


tbb::concurrent_queue<task> jobs_priority_1;
tbb::concurrent_hash_map<pid_t, std::list<task_finished>> tasks_finished_map;
#endif

void signal_handler(int sig){
	switch(sig) {
		case SIGHUP: 
			//log_message(LOG_FILE, "Hangup signal catched");
			break;
		case SIGTERM:
			//log_message(LOG_FILE, "Terminate signal catched");
			//log_message(LOG_FILE, "Shutting down urd");
			exit(EXIT_SUCCESS);
			break;
		default:
			//log_message(LOG_FILE, "Unknown signal received");
			break;
		}	
}

void urd::daemonize() {
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

	pid_t pid, sid;

	/* Check if this is already a daemon */ 
	if (getpid() == 1){
		return;
	}
	
	/* Fork off the parent process */
	pid = fork();

	/* Fork error */
	if (pid < 0) {
		m_logger->error("[daemonize] fork failed.");
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
		m_logger->error("[daemonize] setsid failed.");
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
		m_logger->error("[daemonize] dup[1] failed.");
		perror("dup");
		exit(EXIT_FAILURE);
	}
	if(-1 == dup(i)){ /* stderr */
		m_logger->error("[daemonize] dup[2] failed.");
		perror("dup");
		exit(EXIT_FAILURE);
	}

	/* Change the file mode mask */
	umask(027); /* file creation mode to 750 */

	/* Change the current working directory */
	if ((chdir(m_settings->m_running_dir.c_str())) < 0) {
		m_logger->error("[daemonize] chdir failed.");
		perror("Chdir");
		exit(EXIT_FAILURE);
	}
	
	/* Check if daemon already exists:
	 * First instance of the daemon will lock the file so that other
	 * instances understand that an instnace is already running. 
	 */

	int lfp;
	lfp = open(m_settings->m_daemon_pidfile.c_str(), O_RDWR|O_CREAT, 0640);
	if(lfp < 0){
		m_logger->error("[daemonize] can not open daemon lock file");
		perror("Can not open daemon lock file");
		exit(EXIT_FAILURE);
	} 
	if(lockf(lfp, F_TLOCK, 0)<0){
		m_logger->error("[daemonize] another instance of this daemon already running");
		perror("Another instance of this daemon already running");
		exit(EXIT_FAILURE);
	}

	/* record pid in lockfile */
	char str[10];
	size_t err_snprintf;
	err_snprintf = snprintf(str, sizeof(str), "%d\n", getpid());
	if(err_snprintf >= sizeof(str)){
		m_logger->error("[daemonize] snprintf failed");
	}
	size_t err_write;
	err_write = write(lfp, str, strnlen(str, sizeof(str)));
	if(err_write != strnlen(str, sizeof(str))){
		m_logger->error("[daemonize] write failed");
	}

	/* Manage signals */
	signal(SIGCHLD, SIG_IGN); /* ignore child */
	signal(SIGTSTP, SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGHUP, signal_handler); /* catch hangup signal */
	signal(SIGTERM, signal_handler); /* catch kill signal */

}

#if 0
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


int urd::server_init(sock_ev_serv *serv, const char *sock_path){
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


/*static*/ void urd::read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents){
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


/*inline static*/ urd::sock_ev_client urd::client_new(int fd){
    //FIXME BUG: returning reference from local variable
	sock_ev_client client;
	client.fd = fd;
	set_non_block(client.fd);
	//ev_io_init(&client.io, client_cb, client.fd, EV_READ);
	log_message(LOG_FILE, "client_new registered");
	return client;
}

/*static*/ void urd::accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents){
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
#endif

#if 0
void urd::communication_thread(int id){
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
#endif

#if 0
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
#endif

void urd::new_request_handler(struct norns_iotd* iotdp){

    /* create a task descriptor & modify original with the task's assigned ID */
    //std::unique_ptr<urd::task> task(new urd::task(iotdp)); 
    //iotdp->ni_tid = task->m_task_id;

    m_workers->push(urd::task(iotdp));

    /* the ipc_listener will automatically reply to the client when we exit the handler */
}


void urd::set_configuration(const config_settings& settings) {
    m_settings = std::make_shared<config_settings>(settings);
}

void urd::run() {

    // initialize logging facilities
    m_logger = std::shared_ptr<logger>(new logger(m_settings->m_progname, "stdout"));

	m_logger->info("===========================");
	m_logger->info("== Starting Urd daemon   ==");
	m_logger->info("===========================");

	m_logger->info("");
	m_logger->info("* Settings:");
	m_logger->info("    daemonize?: {}", m_settings->m_daemonize);
	m_logger->info("    running directory: {}", m_settings->m_running_dir);
	m_logger->info("    pidfile: {}", m_settings->m_daemon_pidfile);
	m_logger->info("    workers: {}", m_settings->m_workers_in_pool);
	m_logger->info("    internal storage: {}", m_settings->m_storage_path);
	m_logger->info("    internal storage capacity: {}", m_settings->m_storage_capacity);

    //daemonize();
    
    // create worker pool
    m_workers = std::shared_ptr<ctpl::thread_pool>(new ctpl::thread_pool(m_settings->m_workers_in_pool));
    m_logger->info("* Creating workers...");

    // create (but not start) the IPC listening mechanism
    m_logger->info("* Creating request listener...");
    ::unlink(m_settings->m_ipc_sockfile.c_str());
    m_ipc_listener = std::shared_ptr<ipc_listener<struct norns_iotd>>(
        new ipc_listener<struct norns_iotd>(m_settings->m_ipc_sockfile,
                std::bind(&urd::new_request_handler, this, std::placeholders::_1)));

    m_logger->info("Urd daemon successfully started!");
    m_logger->info("Awaiting requests...");
    m_ipc_listener->run();
	/*
	 *	Create thread pool
	 *	Set number of threads
	 *	Start infinite loop
	 */

	//ctpl::thread_pool p(N_THREADS_IN_POOL);
	//p.push(communication_thread);
	//push_jobs(p);
	
	//m_workers.push(urd::communication_thread);
}
