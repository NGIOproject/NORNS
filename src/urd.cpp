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
#include "backends.hpp"
#include "logger.hpp"
#include "job.hpp"
#include "resources.hpp"
#include "io-task.hpp"
#include "urd.hpp"
#include "make-unique.hpp"

pid_t urd::daemonize() {
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
        return 0;
	}

	/* Fork off the parent process */
	if((pid = fork()) < 0) {
		LOGGER_ERROR("[daemonize] fork failed.");
		perror("Fork");
		exit(EXIT_FAILURE);
	}

	/* Parent exits */
	if(pid != 0) {
		return pid;
	}

	/* Obtain new process group */
	if((sid = setsid()) < 0) {
		/* Log failure */
		LOGGER_ERROR("[daemonize] setsid failed.");
		perror("Setsid");
		exit(EXIT_FAILURE);
	}

	/* Close all descriptors */
	for(int i = getdtablesize() - 1; i >= 0; --i){
		close(i);
	} 

	/* Handle standard IO */
	int fd = open("/dev/null", O_RDWR); /* open stdin */

	if(dup(fd) == -1) { /* stdout */
		LOGGER_ERROR("[daemonize] dup[1] failed.");
		perror("dup");
		exit(EXIT_FAILURE);
	}

	if(dup(fd) == -1) { /* stderr */
		LOGGER_ERROR("[daemonize] dup[2] failed.");
		perror("dup");
		exit(EXIT_FAILURE);
	}

	/* Change the file mode mask */
	umask(027); /* file creation mode to 750 */

	/* Change the current working directory */
	if(chdir(m_settings->m_running_dir.c_str()) < 0) {
		LOGGER_ERROR("[daemonize] chdir failed.");
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
		LOGGER_ERROR("[daemonize] can not open daemon lock file");
		perror("Can not open daemon lock file");
		exit(EXIT_FAILURE);
	} 

	if(lockf(lfp, F_TLOCK, 0) < 0) {
		LOGGER_ERROR("[daemonize] another instance of this daemon already running");
		perror("Another instance of this daemon already running");
		exit(EXIT_FAILURE);
	}

	/* record pid in lockfile */
	char str[10];
	size_t err_snprintf;
	err_snprintf = snprintf(str, sizeof(str), "%d\n", getpid());

	if(err_snprintf >= sizeof(str)) {
		LOGGER_ERROR("[daemonize] snprintf failed");
	}

	size_t err_write;
	err_write = write(lfp, str, strnlen(str, sizeof(str)));

	if(err_write != strnlen(str, sizeof(str))) {
		LOGGER_ERROR("[daemonize] write failed");
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

norns_error_t urd::validate_iotask_args(norns_op_t op, 
                                        const resource_info_ptr src_info,
                                        const resource_info_ptr dst_info) const {

    if(op != NORNS_IOTASK_COPY && op != NORNS_IOTASK_MOVE) {
        return NORNS_EBADARGS;
    }

    // src_resource cannot be remote
    if(src_info->type() == data::resource_type::remote_posix_path) {
        return NORNS_ENOTSUPPORTED;
    }

    // dst_resource cannot be a memory region
    if(dst_info->type() == data::resource_type::memory_region) {
        return NORNS_ENOTSUPPORTED;
    }

    return NORNS_SUCCESS;
}

response_ptr urd::create_task(const request_ptr base_request) {


    // downcast the generic request to the concrete implementation
    auto request = static_cast<api::transfer_task_request*>(base_request.get());

    auto type = request->get<0>();
    auto src_info = request->get<1>();
    auto dst_info = request->get<2>();

    response_ptr resp;
    norns_tid_t tid = 0;
    norns_error_t rv = NORNS_SUCCESS;

    /* Helper lambda used to construct a data::resource from a storage::backend and 
     * a data::resource_info. The function creates a data::resource if the provided 
     * data::resource_info 'info' is valid, return it
     * XXX
     * in the 'res' parameter and return NORNS_SUCCESS. Otherwise, return an appropriate error */
    auto make_resource = [&](const resource_info_ptr info) -> std::pair<norns_error_t, resource_ptr> {

        // remote backend validation is left to the recipient
        if(info->is_remote()) {

            return std::make_pair<norns_error_t, resource_ptr>(NORNS_SUCCESS, 
                    std::make_shared<data::resource>(storage::remote_backend, info)); //XXX we need a "remote" backend
        }

        const auto nsid = info->nsid();

        // backend does not exist 
        if(m_backends->count(nsid) == 0) {
            return std::make_pair(NORNS_ENOSUCHBACKEND, resource_ptr());
        }

        // check that resource is compatible with selected backend
        auto backend_ptr = m_backends->at(nsid);

        if(!backend_ptr->accepts(info)) {
            return std::make_pair(NORNS_EBADARGS, resource_ptr());
        }

        return std::make_pair(NORNS_SUCCESS, 
                std::make_shared<data::resource>(backend_ptr, info));
    };

    if((rv = validate_iotask_args(type, src_info, dst_info)) == NORNS_SUCCESS) {

        auto src_rv = make_resource(src_info);
        auto dst_rv = make_resource(dst_info);

        if((src_rv.first == NORNS_SUCCESS) && (dst_rv.first == NORNS_SUCCESS)) {

            resource_ptr src = src_rv.second;
            resource_ptr dst = dst_rv.second;

            // everything is ok, add the I/O task to the queue
            io::task t(type, src, dst);
            tid = t.id();

            m_workers->submit_and_forget(std::move(t));
        }
        else if(src_rv.first != NORNS_SUCCESS) {
            rv = src_rv.first;
        }
        else {
            rv = dst_rv.first;
        }
    }

    resp = std::make_unique<api::transfer_task_response>(tid);
    resp->set_status(rv);

    LOGGER_INFO("NEW_IOTASK({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::ping_request(const request_ptr /*base_request*/) {
    response_ptr resp = std::make_unique<api::ping_response>();

    resp->set_status(NORNS_SUCCESS);

    LOGGER_INFO("PING_REQUEST() = {}", resp->to_string());
    return resp;
}


response_ptr urd::register_job(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::job_register_response>();

    // downcast the generic request to the concrete implementation
    auto request = static_cast<api::job_register_request*>(base_request.get());

    uint32_t jobid = request->get<0>();
    auto hosts = request->get<1>();

    {
        boost::unique_lock<boost::shared_mutex> lock(m_jobs_mutex);

        if(m_jobs.find(jobid) != m_jobs.end()) {
            resp->set_status(NORNS_EJOBEXISTS);
        }
        else {
            m_jobs.emplace(jobid, std::make_shared<job>(jobid, hosts));
            resp->set_status(NORNS_SUCCESS);
        }
    }

    LOGGER_INFO("REGISTER_JOB({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::update_job(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::job_update_response>();

    // downcast the generic request to the concrete implementation
    auto request = static_cast<api::job_update_request*>(base_request.get());

    uint32_t jobid = request->get<0>();
    auto hosts = request->get<1>();

    {
        boost::unique_lock<boost::shared_mutex> lock(m_jobs_mutex);

        const auto& it = m_jobs.find(jobid);

        if(it == m_jobs.end()) {
            resp->set_status(NORNS_ENOSUCHJOB);
        }
        else {
            it->second->update(hosts);
            resp->set_status(NORNS_SUCCESS);
        }
    }

    LOGGER_INFO("UPDATE_JOB({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::remove_job(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::job_unregister_response>();

    // downcast the generic request to the concrete implementation
    auto request = static_cast<api::job_unregister_request*>(base_request.get());

    uint32_t jobid = request->get<0>();

    {
        boost::unique_lock<boost::shared_mutex> lock(m_jobs_mutex);

        const auto& it = m_jobs.find(jobid);

        if(it == m_jobs.end()) {
            resp->set_status(NORNS_ENOSUCHJOB);
        }
        else {
            m_jobs.erase(it);
            resp->set_status(NORNS_SUCCESS);
        }
    }

    LOGGER_INFO("UNREGISTER_JOB({}) = {}", request->to_string(), resp->to_string());
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
    LOGGER_INFO("ADD_PROCESS({}) = {}", request->to_string(), resp->to_string());
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
    LOGGER_INFO("REMOVE_PROCESS({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::register_backend(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::backend_register_response>();

    // downcast the generic request to the concrete implementation
    auto request = static_cast<api::backend_register_request*>(base_request.get());

    std::string nsid = request->get<0>();
    int32_t type = request->get<1>();
    std::string mount = request->get<2>();
    int32_t quota = request->get<3>();

    {
        boost::unique_lock<boost::shared_mutex> lock(m_backends_mutex);

        if(m_backends->count(nsid) != 0) {
            resp->set_status(NORNS_EBACKENDEXISTS);
        }
        else {

            backend_ptr bptr = storage::backend_factory::create_from(type, mount, quota);

            if(bptr != nullptr) {
                m_backends->emplace(std::make_pair(nsid, bptr));
                resp->set_status(NORNS_SUCCESS);
            }
            else {
                resp->set_status(NORNS_EBADARGS);
            }
        }
    }

    LOGGER_INFO("REGISTER_BACKEND({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

/* XXX not supported yet
response_ptr urd::update_backend(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::backend_update_response>();

    // downcast the generic request to the concrete implementation
    auto request = static_cast<api::backend_update_request*>(base_request.get());

    resp->set_status(NORNS_SUCCESS);

log_and_return:
    LOGGER_INFO("UPDATE_BACKEND({}) = {}", request->to_string(), resp->to_string());
    return resp;
}*/

response_ptr urd::remove_backend(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::backend_unregister_response>();

    // downcast the generic request to the concrete implementation
    auto request = static_cast<api::backend_unregister_request*>(base_request.get());

    std::string nsid = request->get<0>();

    {
        boost::unique_lock<boost::shared_mutex> lock(m_backends_mutex);

        const auto& it = m_backends->find(nsid);

        if(it == m_backends->end()) {
            resp->set_status(NORNS_ENOSUCHBACKEND);
        }
        else {
            m_backends->erase(it);
            resp->set_status(NORNS_SUCCESS);
        }
    }

    LOGGER_INFO("UNREGISTER_BACKEND({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::unknown_request(const request_ptr /*base_request*/) {
    response_ptr resp = std::make_unique<api::bad_request_response>();

    resp->set_status(NORNS_EBADREQUEST);

    LOGGER_INFO("UNKNOWN_REQUEST() = {}", resp->to_string());
    return resp;
}

void urd::configure(const config_settings& settings) {
    m_settings = std::make_shared<config_settings>(settings);
}

void urd::signal_handler(int signum){

    switch(signum) {

        case SIGINT:
            LOGGER_WARN("A signal (SIGINT) occurred.");
            if(m_api_listener) {
                m_api_listener->stop();
            }
            break;

        case SIGTERM:
            LOGGER_WARN("A signal (SIGTERM) occurred.");
            if(m_api_listener) {
                m_api_listener->stop();
            }
            break;

        case SIGHUP:
            LOGGER_WARN("A signal (SIGHUP) occurred.");
            break;
    }
}

int urd::run() {

    // initialize logging facilities
    if(m_settings->m_daemonize) {
        logger::create_global_logger(m_settings->m_progname, "syslog");

        if(daemonize() != 0) {
            /* parent clean ups and unwinds */
            teardown();
            return EXIT_SUCCESS;
        }
    } else {
        if(m_settings->m_use_syslog) {
            logger::create_global_logger(m_settings->m_progname, "syslog");
        }
        else {
            logger::create_global_logger(m_settings->m_progname, "console color");
        }
    }

	LOGGER_INFO("===========================");
	LOGGER_INFO("== Starting Urd daemon   ==");
	LOGGER_INFO("===========================");

	LOGGER_INFO("");
	LOGGER_INFO("* Settings:");
	LOGGER_INFO("    daemonize?: {}", m_settings->m_daemonize);
	LOGGER_INFO("    running directory: {}", m_settings->m_running_dir);
	LOGGER_INFO("    pidfile: {}", m_settings->m_daemon_pidfile);
	LOGGER_INFO("    workers: {}", m_settings->m_workers_in_pool);
	LOGGER_INFO("    internal storage: {}", m_settings->m_storage_path);
	LOGGER_INFO("    internal storage capacity: {}", m_settings->m_storage_capacity);

    // signal handlers must be installed AFTER daemonizing
    LOGGER_INFO("* Installing signal handlers...");
    m_signal_listener = std::make_unique<signal_listener>(std::bind(&urd::signal_handler, this, std::placeholders::_1));

    m_signal_listener->run();

    // create worker pool
    LOGGER_INFO("* Creating workers...");
    m_workers = std::make_unique<thread_pool>(m_settings->m_workers_in_pool);

    // create (but not start) the API listening mechanism
    LOGGER_INFO("* Creating request listener...");
    ::unlink(m_settings->m_ipc_sockfile.c_str());

	// temporarily change the umask so that the socket file can be accessed by anyone
	mode_t old_mask = umask(0111);

    // create API listener and register callbacks for each request type
    m_api_listener = std::make_unique<api_listener>(m_settings->m_ipc_sockfile);

    /* user-level functionalities */
    m_api_listener->register_callback(
            api::request_type::transfer_task,
            std::bind(&urd::create_task, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::ping,
            std::bind(&urd::ping_request, this, std::placeholders::_1));

    /* admin-level functionalities */
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
            api::request_type::backend_register,
            std::bind(&urd::register_backend, this, std::placeholders::_1));

/*    m_api_listener->register_callback(
            api::request_type::backend_update,
            std::bind(&urd::update_backend, this, std::placeholders::_1));*/

    m_api_listener->register_callback(
            api::request_type::backend_unregister,
            std::bind(&urd::remove_backend, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::bad_request,
            std::bind(&urd::unknown_request, this, std::placeholders::_1));

    m_backends = std::make_unique<backend_manager>();

    // restore the umask
	umask(old_mask);

    LOGGER_INFO("Urd daemon successfully started!");
    LOGGER_INFO("Awaiting requests...");
    m_api_listener->run();

	LOGGER_INFO("");
	LOGGER_INFO("===========================");
	LOGGER_INFO("== Stopping Urd daemon   ==");
	LOGGER_INFO("===========================");
	LOGGER_INFO("");

    teardown();

	LOGGER_INFO("");
	LOGGER_INFO("[Stopped]");

    return EXIT_SUCCESS;
}

void urd::teardown() {

    if(m_signal_listener) {
        LOGGER_INFO("* Stopping signal listener...");
        m_signal_listener->stop();
        //m_signal_listener.reset();
    }

    if(m_api_listener) {
        LOGGER_INFO("* Stopping API listener...");
        m_api_listener->stop();
        m_api_listener.reset();
    }

    api_listener::cleanup();

    if(m_settings) {
        ::unlink(m_settings->m_daemon_pidfile.c_str());
        m_settings.reset();
    }

    //m_workers.reset();
    if(m_workers) {
        LOGGER_INFO("* Stopping worker threads...");
        m_workers->stop();
        m_workers.reset();
    }
}
