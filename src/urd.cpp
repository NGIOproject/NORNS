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
#include <ctime>

#include <boost/atomic.hpp>
#include <functional>



#include <norns.h>

#include "ipc-listener.hpp"
#include "signal-listener.hpp"
#include "backends.hpp"
#include "ctpl.h" 
#include "logger.hpp"
#include "urd.hpp"

#if 0 
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
	 * instances understand that an instance is already running. 
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
//	signal(SIGHUP, signal_handler); /* catch hangup signal */
//	signal(SIGTERM, signal_handler); /* catch kill signal */

}

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

void urd::signal_handler(int signum){

    switch(signum) {

        case SIGTERM:
            m_logger->info(" A signal(SIGTERM) occurred.");

            m_ipc_listener->stop();
            ::unlink(m_settings->m_daemon_pidfile.c_str());
            m_logger.reset();
            break;

        case SIGHUP:
            m_logger->info(" A signal(SIGHUP) occurred.");
            break;
    }
}

void urd::run() {

    // initialize logging facilities
    if(m_settings->m_daemonize) {
        m_logger = std::shared_ptr<logger>(new logger(m_settings->m_progname, "syslog"));
    } else{
        m_logger = std::shared_ptr<logger>(new logger(m_settings->m_progname, "stdout color"));
    }

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

    if(m_settings->m_daemonize) {
        daemonize();
    }

    // instantiate configured backends
    m_logger->info("* Creating storage backend handlers...");
    for(const auto& bend : m_settings->m_backends){
        try {

            auto b = storage::backend_factory::get_instance().create(bend.m_type, bend.m_options);
            m_backends.push_back(b);

            m_logger->info("    Registered backend '{}' (type: {})", bend.m_name, bend.m_type);  
            m_logger->info("      [ capacity: {} bytes ]", b->get_capacity());

        } catch(std::invalid_argument ex) {
            m_logger->warn(" Ignoring definition of backend '{}' (type: unknown)", bend.m_name);
        }
    }

    // signal handlers must be installed AFTER daemonizing
    m_logger->info("* Installing signal handlers...");
    m_signal_listener = std::shared_ptr<signal_listener>(new signal_listener(
                std::bind(&urd::signal_handler, this, std::placeholders::_1)));

    m_signal_listener->run();

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
