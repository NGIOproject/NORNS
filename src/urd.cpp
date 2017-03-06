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

#include "ctpl.h" 

<<<<<<< HEAD:src/urd.cpp
#define SOCKET_NAME "urd_socket"    
=======
#define RUNNING_DIR "/tmp"  
#define SOCKET_FILE "/tmp/dloom.socket"  
#define DAEMON_LOCK_FILE "/tmp/dloom.lock"
#define LOG_FILE "tmp/dloom.log"
>>>>>>> bf32803ae612688c523f8fbbbd39d631a095a700:src/dloom.cpp

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

tbb::concurrent_queue<task> jobs_priority_1;
tbb::concurrent_hash_map<pid_t, std::list<task_finished>> tasks_finished_map;

void log_message(const char filename[], const char message[]){
	FILE *logfile;
	logfile=fopen(filename,"a");
	if(!logfile) return;
	fprintf(logfile,"%s\n",message);
	fclose(logfile);
}

void signal_handler(int sig){
	switch(sig) {
	case SIGHUP: 
		log_message(LOG_FILE, "Hangup signal catched");
		break;
	case SIGTERM:
		log_message(LOG_FILE,"terminate signal catched");
		exit(0);
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
		return;
	}

<<<<<<< HEAD:src/urd.cpp
	/* Change the file mode mask */
	umask(0);
	
	/* Check if urd already exists:
	 * 1. Make sure folder exists /var/run/urd
	 * 2. Check if file urd.txt exists
	 */

	struct stat sb;
	if (stat("/var/run/urd", &sb) == 0 && S_ISDIR(sb.st_mode)){
	    std::cout << "directory already existed" << std::endl;
	} else {
		/* Directory does not exist */
		if(mkdir("/var/run/urd", 0700)== -1){
			std::cout << "[urd][base_initialization] Error mkdir. Check privilages under /var/run/*" << std::endl;
		}
	}
	
	if (access("/var/run/urd/urd.txt", F_OK) != -1) {
		/* An instance of urd already exists. */
		perror("An instance of urd already exists. Exiting");
		exit(1);
	} else {
		/* First instance of urd, create urd.txt so no other instance may be initialized */
		std::ofstream outfile("/var/run/urd/urd.txt");
		outfile << pid << std::endl;		
	}

	/* Create a new SID for the child process */
=======
	/* Obtain new process group */
>>>>>>> bf32803ae612688c523f8fbbbd39d631a095a700:src/dloom.cpp
	sid = setsid();
	if (sid < 0) {
		/* Log failure */
		throw std::exception();
	}

	/* Close all descriptors */
	int i;
	for(i=getdtablesize(); i>=0; --i){
		if(close(i)<0){
			perror("Close descritpors");
			exit(1);
		}	
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

void comm_thread(int id){
	/*
	 * 1. Initialize socket
	 * 2. Check if incoming rpc or status rpc
	 * 	if (incoming rpc):
	 *		check prioritiy and enqueu
	 *	else if (status rpc) :
	 *		check status and return
	 * 3. Back to 2. 
	 */

	/* Variables necessary for the socket */
	int sock, msgsock, rval, listen_err;
	struct sockaddr_un server;
	char buf[1024];

	/* Create socket */
	sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
        perror("opening stream socket");
        exit(1);
    }

    /* Name and bind the socket using file system name */
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, SOCKET_NAME);
    if (bind(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un))) {
    	perror("binding stream obert");
    	exit(1);
    }

    /* Start accepting connections, and start reading on the accepted condition */
    listen_err = listen(sock, 10);
    if (listen_err == -1){
    	perror("listen socket failed");
    	exit(1);
    }

	job_type type;
	uint64_t taskId;
	while(1){

		msgsock = accept(sock, 0, 0);
		if (msgsock == -1)
			perror("accept");
		else do {

			struct foo bar;
			memset(&bar, 0, sizeof(bar));

			rval = read(msgsock, &bar, sizeof(bar));
			continue;

			bzero(buf, sizeof(buf));
			if ((rval = read(msgsock, buf, 1024)) > 0){
				/*
				 * Transform buf to job_type.
				 * Switch through the different cases.
				 */
				task t;
				type = enqueue_task;
				switch(type){
					case enqueue_task:
					{
						// sleep(2);
						// incoming rpc
						// set request id
						jobs_priority_1.push(t);
						// return request id through pipe
						break;
					}
					case check_task_status:
					{
						// return certain job id status
						pid_t pid_thread;
						tbb::concurrent_hash_map<pid_t, std::list<task_finished>>::accessor acc;
						if(tasks_finished_map.find(acc, pid_thread)){
							//key pid_thread is found
							for(auto iterator = acc->second.begin(); iterator !=acc->second.end();){
								if (iterator->taskId == taskId){
									//found the task, should erase it and return it. 
									uint64_t status = iterator->status; 
									iterator = acc->second.erase(iterator); 
									//retornar status

									//break
									break;
								} else {
									 ++iterator;
								}
							}
						} else {
							//key pid_thread is not found in map, it may be processing or waiting to be processed
						}
						break;
					}
					case check_all_tasks:
					{
						// return all jobs from certain pid_t
						pid_t pid_thread;
						tbb::concurrent_hash_map<pid_t, std::list<task_finished>>::accessor acc;
						if(tasks_finished_map.find(acc, pid_thread)){
							//key pid_is found, return all jobs inside
							//while list not empty, pop and return job
							while(!acc->second.empty()){
								task_finished tf = acc->second.front();
								acc->second.pop_front();
								//return tf
							}
						} else{
							//no jobs from pid_thread are finished yet.
						}

						break;
					}
				}

			} else if (rval == 0)
				printf("Ending connection\n");
			else
				perror("reading stream message");
		} while (rval > 0);
		close(msgsock);
	}

	close(sock);
    unlink(SOCKET_NAME);

}


void serve_job(int id, task t){
	/*
	 * 1. Serve t
	 * 2. Enqueue t to finished queue
	 */

	//serve
	//enqueue
}


void push_jobs(ctpl::thread_pool &p){
	/*
	 * 1. If p.n_idle > 0 (probably sleep between checks)
	 * 2. Push job
	 */

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

	}

}

void infinite_loop() {
	/*
	 *	Create thread pool
	 *	Set number of threads
	 *	Start infinite loop
	 */

	ctpl::thread_pool p(3);
	p.push(comm_thread);
	push_jobs(p);
}

int main() {
	daemonize();
	infinite_loop();   
}
