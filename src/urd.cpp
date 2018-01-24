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
#include <sys/wait.h>
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

#include <norns-rpc.h>
#include <norns.h>

#include "api.hpp"
#include "signal-listener.hpp"
#include "backend-base.hpp"
#include "ctpl.h" 
#include "logger.hpp"
#include "job.hpp"
#include "io-task.hpp"
#include "urd.hpp"
#include "make-unique.hpp"

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
	if(getppid() == 1) {
	    m_pid = getpid();
        return;
	}

	/* Fork off the parent process */
	if((pid = fork()) < 0) {
		m_logger->error("[daemonize] fork failed.");
		perror("Fork");
		exit(EXIT_FAILURE);
	}

    m_pid = pid;

	/* Parent exits */
	if(pid > 0) {
		return;
	}

	/* Obtain new process group */
	if((sid = setsid()) < 0) {
		/* Log failure */
		m_logger->error("[daemonize] setsid failed.");
		perror("Setsid");
		exit(EXIT_FAILURE);
	}

	/* Close all descriptors */
	for(int i = getdtablesize(); i >= 0; --i){
		close(i);
	} 

	/* Handle standard IO */
	int fd = open("/dev/null", O_RDWR); /* open stdin */

	if(dup(fd) == -1) { /* stdout */
		m_logger->error("[daemonize] dup[1] failed.");
		perror("dup");
		exit(EXIT_FAILURE);
	}

	if(dup(fd) == -1) { /* stderr */
		m_logger->error("[daemonize] dup[2] failed.");
		perror("dup");
		exit(EXIT_FAILURE);
	}

	/* Change the file mode mask */
	umask(027); /* file creation mode to 750 */

	/* Change the current working directory */
	if(chdir(m_settings->m_running_dir.c_str()) < 0) {
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

	if(lfp < 0) {
		m_logger->error("[daemonize] can not open daemon lock file");
		perror("Can not open daemon lock file");
		exit(EXIT_FAILURE);
	} 

	if(lockf(lfp, F_TLOCK, 0) < 0) {
		m_logger->error("[daemonize] another instance of this daemon already running");
		perror("Another instance of this daemon already running");
		exit(EXIT_FAILURE);
	}

	/* record pid in lockfile */
	char str[10];
	size_t err_snprintf;
	err_snprintf = snprintf(str, sizeof(str), "%d\n", getpid());

	if(err_snprintf >= sizeof(str)) {
		m_logger->error("[daemonize] snprintf failed");
	}

	size_t err_write;
	err_write = write(lfp, str, strnlen(str, sizeof(str)));

	if(err_write != strnlen(str, sizeof(str))) {
		m_logger->error("[daemonize] write failed");
	}

	close(lfp);

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

response_ptr urd::register_job(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::job_register_response>();

    // downcast the generic request to the concrete implementation
    auto request = static_cast<api::job_register_request*>(base_request.get());

    uint32_t jobid = request->get<0>();
    auto hosts = request->get<1>();

    boost::unique_lock<boost::shared_mutex> lock(m_jobs_mutex);

    if(m_jobs.find(jobid) != m_jobs.end()) {
        resp->set_status(NORNS_EJOBEXISTS);
        goto log_and_return;
    }

    m_jobs.emplace(jobid, 
                   std::make_shared<job>(jobid, hosts));

    resp->set_status(NORNS_SUCCESS);

log_and_return:
    m_logger->info("REGISTER_JOB({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::update_job(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::job_update_response>();

    // downcast the generic request to the concrete implementation
    auto request = static_cast<api::job_update_request*>(base_request.get());

    uint32_t jobid = request->get<0>();
    auto hosts = request->get<1>();

    boost::unique_lock<boost::shared_mutex> lock(m_jobs_mutex);

    const auto& it = m_jobs.find(jobid);

    if(it == m_jobs.end()) {
        resp->set_status(NORNS_ENOSUCHJOB);
        goto log_and_return;
    }

    it->second->update(hosts);

    resp->set_status(NORNS_SUCCESS);

log_and_return:
    m_logger->info("UPDATE_JOB({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::remove_job(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::job_unregister_response>();

    // downcast the generic request to the concrete implementation
    auto request = static_cast<api::job_unregister_request*>(base_request.get());

    uint32_t jobid = request->get<0>();

    boost::unique_lock<boost::shared_mutex> lock(m_jobs_mutex);

    const auto& it = m_jobs.find(jobid);

    if(it == m_jobs.end()) {
        resp->set_status(NORNS_ENOSUCHJOB);
        goto log_and_return;
    }

    m_jobs.erase(it);

    resp->set_status(NORNS_SUCCESS);

log_and_return:
    m_logger->info("UNREGISTER_JOB({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::add_process(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::process_register_response>();

    // downcast the generic request to the concrete implementation
    auto request = static_cast<api::process_register_request*>(base_request.get());

    uint32_t jobid = request->get<0>();
    pid_t uid = request->get<1>();
    gid_t gid = request->get<2>();
    pid_t pid = request->get<3>();

    boost::unique_lock<boost::shared_mutex> lock(m_jobs_mutex);

    const auto& it = m_jobs.find(jobid);

    if(it == m_jobs.end()) {
        resp->set_status(NORNS_ENOSUCHJOB);
        goto log_and_return;
    }

    it->second->add_process(pid, gid);

    resp->set_status(NORNS_SUCCESS);

log_and_return:
    m_logger->info("ADD_PROCESS({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::remove_process(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::process_unregister_response>();

    // downcast the generic request to the concrete implementation
    auto request = static_cast<api::process_unregister_request*>(base_request.get());

    uint32_t jobid = request->get<0>();
    pid_t uid = request->get<1>();
    gid_t gid = request->get<2>();
    pid_t pid = request->get<3>();

    boost::unique_lock<boost::shared_mutex> lock(m_jobs_mutex);

    const auto& it = m_jobs.find(jobid);

    if(it == m_jobs.end()) {
        resp->set_status(NORNS_ENOSUCHJOB);
        goto log_and_return;
    }

    if(it->second->find_and_remove_process(pid, gid)) {
        resp->set_status(NORNS_SUCCESS);
    }
    else {
        resp->set_status(NORNS_ENOSUCHPROCESS);
    }

log_and_return:
    m_logger->info("REMOVE_PROCESS({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::create_task(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::transfer_task_response>();

    // downcast the generic request to the concrete implementation
    auto request = static_cast<api::transfer_task_request*>(base_request.get());

    m_workers->push(io::task());

    resp->set_status(NORNS_SUCCESS);

    m_logger->info("CREATE_TASK({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

void urd::configure(const config_settings& settings) {
    m_settings = std::make_shared<config_settings>(settings);
}

void urd::signal_handler(int signum){

    switch(signum) {

        case SIGINT:
            m_logger->info(" A signal(SIGINT) occurred.");
            break;

        case SIGTERM:
            m_logger->info(" A signal(SIGTERM) occurred.");

            m_api_listener->stop();
            ::unlink(m_settings->m_daemon_pidfile.c_str());
            m_logger.reset();
            exit(EXIT_SUCCESS);
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

        daemonize();

        if(m_settings->m_detach) {
            exit(EXIT_SUCCESS);
        }

        if(m_pid != 0) {
            return;
        }
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

    // instantiate configured backends
//    m_logger->info("* Creating storage backend handlers...");
//    for(const auto& bend : m_settings->m_backends){
//        try {
//
//            auto b = storage::backend_factory::get_instance().create(bend.m_type, bend.m_options);
//            m_backends.push_back(b);
//
//            m_logger->info("    Registered backend '{}' (type: {})", bend.m_name, bend.m_type);  
//            m_logger->info("      [ capacity: {} bytes ]", b->get_capacity());
//
//        } catch(std::invalid_argument ex) {
//            m_logger->warn(" Ignoring definition of backend '{}' (type: unknown)", bend.m_name);
//        }
//    }

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

	// temporarily change the umask so that the socket file can be accessed by anyone
	mode_t old_mask = umask(0111);

    // create API listener and register callbacks for each request type
    m_api_listener = std::make_unique<api_listener>(m_settings->m_ipc_sockfile);

    m_api_listener->register_callback(
            api::request_type::job_register,
            std::bind(&urd::register_job, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::job_update,
            std::bind(&urd::update_job, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::job_unregister,
            std::bind(&urd::remove_job, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::process_register,
            std::bind(&urd::add_process, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::process_unregister,
            std::bind(&urd::remove_process, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::transfer_task,
            std::bind(&urd::create_task, this, std::placeholders::_1));

    // restore the umask
	umask(old_mask);

    m_logger->info("Urd daemon successfully started!");
    m_logger->info("Awaiting requests...");
    m_api_listener->run();
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

void urd::stop() {
    if(m_pid != 0) {

        m_logger->info("[stop] Sending SIGTERM to {}...", m_pid);

        if(kill(m_pid, SIGTERM) != 0) {
            m_logger->error("[stop] Unable to send SIGTERM to daemon process: {}", strerror(errno));
            return;
        }

        int status;

        if(waitpid(m_pid, &status, 0) == -1) {
            m_logger->error("[stop] Unable to wait for daemon process: {}", strerror(errno));
            return;
        }

        m_logger->info("[stop] Daemon process exited with status {}", status);
    }
}

