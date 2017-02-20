// g++  -I/usr/local/ -std=c++11 -o dloom2 dloom2.cpp -L tbb2017_20161128oss/lib/intel64/gcc4.7/ -ltbb
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstdlib>
#include <exception>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <fcntl.h>
#include "ctpl.h" 
#include <tbb/tbb.h>

#define SOCKET_NAME "dloom_socket"    

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

void daemon_basic_initialization() {
	/*
	 * --- daemon basic initalization ---
	 * 1. Fork off praent process
	 * 2. Exit parent
	 * 3. Change file mode mask
	 * 4. Check if daemon already exists
	 * 5. Create new SID for the child process
	 * 6. Change current working directory
	 * 7. Close out standart file descriptor
	 */
	pid_t pid, sid;
	
	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		throw std::exception();
	}

	/* If we got a good PID, then
	   we can exit the parent process. */
	if (pid > 0) {
		return;
	}

	/* Change the file mode mask */
	umask(0);
	
	/* Check if dloom already exists:
	 * 1. Make sure folder exists /var/run/dloom
	 * 2. Check if file dloom.txt exists
	 */

	struct stat sb;
	if (stat("/var/run/dloom", &sb) == 0 && S_ISDIR(sb.st_mode)){
	    std::cout << "directory already existed" << std::endl;
	} else {
		/* Directory does not exist */
		if(mkdir("/var/run/dloom", 0700)== -1){
			std::cout << "[dloom][base_initialization] Error mkdir. Check privilages under /var/run/*" << std::endl;
		}
	}
	
	if (access("/var/run/dloom/dloom.txt", F_OK) != -1) {
		/* An instance of dloom already exists. */
		perror("An instance of dloom already exists. Exiting");
		exit(1);
	} else {
		/* First instance of dloom, create dloom.txt so no other instance may be initialized */
		std::ofstream outfile("/var/run/dloom/dloom.txt");
		outfile << pid << std::endl;		
	}

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		/* Log failure */
		throw std::exception();
	}

	/* Change the current working directory */
	if ((chdir("/")) < 0) {
		/* Log failure */
		throw std::exception();
	}

	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
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





void daemon_specific_initialization() {
	/* set number of threads */
	ctpl::thread_pool p(3);
	p.push(comm_thread);
	/* push_jobs may be done by this thread */
	push_jobs(p);
}

int main() {
	daemon_basic_initialization();
	daemon_specific_initialization();   
}
