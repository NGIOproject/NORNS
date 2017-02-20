#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>

/* TO-DO NUM_THREADS will be dynamic and will be setted from norn library */
#define NUM_THREADS		2

pthread_t wid[NUM_THREADS];

/*
 * Master needs a structure to check if workers are available or not. He will be the only one using this structure.
 * Structure of condition variables(one for each worker) for the master to notify when a request has been asigned.
 * 
 */

pthread_mutex_t requests_mutex[NUM_THREADS];
pthread_cond_t requests_threshold[NUM_THREADS];

void* worker()
{
	/*
	 * --- Worker ---
	 * 1. Wait until job has been assigned
	 * 	- Using conditional variables master will notify when a job has been assigned
	 * 2. Do job
	 * 3. Notify master
	 *  - Using mutex variables ? (we can set different values, maybe for errors)
	 * 4. Return to step 1
	 */

	pthread_t id = pthread_self();

	pthread_mutex_lock(&requests_mutex[]);
	while(1){

	}

	return NULL;
}

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
		exit(EXIT_FAILURE);
	}

	/* If we got a good PID, then
	   we can exit the parent process. */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* Change the file mode mask */
	umask(0);
	
	/* Check if dloom already exists:
	 * 1. Make sure folder exists /var/run/dloom
	 * 2. Check if file dloom.txt exists
	 */

	DIR* d_dir = opendir("/var/run/dloom");
	if (d_dir) {
		printf("directory already existed\n");
		/* Directory already exists */
		closedir(d_dir);
	} else if (ENOENT == errno) {
		printf("directory did not exist\n");
		/* Directory does not exist */
		if(mkdir("/var/run/dloom", 0700)== -1){
			printf("[dloom][base_initialization] Error mkdir. Check privilages under /var/run/*\n");
		}
	} else {
		printf("[dloom][base_initialization] Error opendir\n");
	}
	
	if (access("/var/run/dloom/dloom.txt", F_OK) != -1) {
		/* An instance of dloom already exists. */
		printf("An instance of dloom already exists. Exiting\n");
		exit(EXIT_FAILURE);
		
	} else {
		/* First instance of dloom, create dloom.txt so no other instance may be initialized */
		FILE *fp = fopen("/var/run/dloom/dloom.txt", "wb");
		if (fp == NULL) {
			/* Log failure */
			exit(EXIT_FAILURE);
		}
		
	}

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		/* Log failure */
		exit(EXIT_FAILURE);
	}

	/* Change the current working directory */
	if ((chdir("/")) < 0) {
		/* Log failure */
		exit(EXIT_FAILURE);
	}

	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
}

void daemon_specific_initialization() {
	/**
     * --- daemon specific initalization ---
     * 1. Create workers
     * 2. Master loop
     */

    /* Create NUM_THREADSworkers */
    int i, err;
    for (i=0; i<NUM_THREADS; ++i){
    	err = pthread_create(&(wid[i]), NULL, &worker, NULL);
        if (err != 0)
            printf("Can't create thread :[%s]", strerror(err));
        else
            printf("Thread created successfully\n");
    }

    /*
	 * --- Master ---
	 * 1. Check if request is received
	 *  - When req received: 
	 		If worker is available, assign request and notify worker (conditional variables). Else enqueue request
	 * 3. Check if job assigned is finished
	 	- If worker has finished dealing with req:
			Set worker as free, notify app request is done
	 * 4. Check if queue of requests is not empyt:
	 	- If so, check if worker is available and assign it. 
	 * 5. Return to step 1
	 */

	exit(EXIT_SUCCESS);
	
}

int main() {
	daemon_basic_initialization();
	daemon_specific_initialization();   
	return 0;
}
