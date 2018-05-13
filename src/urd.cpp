/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of the NORNS Data Scheduler, a service that allows  *
 * other programs to start, track and manage asynchronous transfers of   *
 * data resources transfers requests between different storage backends. *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The NORNS Data Scheduler is free software: you can redistribute it    *
 * and/or modify it under the terms of the GNU Lesser General Public     *
 * License as published by the Free Software Foundation, either          *
 * version 3 of the License, or (at your option) any later version.      *
 *                                                                       *
 * The NORNS Data Scheduler is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with the NORNS Data Scheduler.  If not, see      *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

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
#include <signal.h>
#include <string.h>
#include <ctime>

#include <boost/atomic.hpp>
#include <functional>

#include "common.hpp"
#include "api.hpp"
#include "backends.hpp"
#include "logger.hpp"
#include "job.hpp"
#include "resources.hpp"
#include "io.hpp"

#include "urd.hpp"

namespace norns {

urd::urd() {}

urd::~urd() {}

pid_t urd::daemonize() {
    /*
     * --- Daemonize structure ---
     *  Check if this is already a daemon
     *  Fork off praent process
     *  Obtain new process group
     *  Close all descriptors
     *  Handle standard IO
     *  Change file mode mask
     *  Change the current working directory
     *  Check if daemon already exists
     *  Manage signals
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

    /* Change the file mode creation mask */
    umask(0);

    /* ensure the process does not keep a directory in use,
     * avoid relative paths beyond this point! */
    if(chdir("/") < 0) {
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
//  signal(SIGHUP, signal_handler); /* catch hangup signal */
//  signal(SIGTERM, signal_handler); /* catch kill signal */

}

urd_error urd::validate_iotask_args(iotask_type type, 
                                    const resource_info_ptr& src_info,
                                    const resource_info_ptr& dst_info) const {

    if(type != iotask_type::copy && type != iotask_type::move) {
        return urd_error::bad_args;
    }

    // src_resource cannot be remote
    if(src_info->type() == data::resource_type::remote_posix_path) {
        return urd_error::not_supported;
    }

    // dst_resource cannot be a memory region
    if(dst_info->type() == data::resource_type::memory_region) {
        return urd_error::not_supported;
    }

    return urd_error::success;
}

response_ptr urd::iotask_create_handler(const request_ptr base_request) {

    // downcast the generic request to the concrete implementation
    auto request = utils::static_unique_ptr_cast<api::iotask_create_request>(std::move(base_request));

    const auto type = request->get<0>();
    const auto src_info = request->get<1>();
    const auto dst_info = request->get<2>();

    response_ptr resp;
    iotask_id tid = 0;
    urd_error rv = urd_error::success;

#if 0
    /* Helper lambda used to construct a data::resource from a *storage::backend* and 
     * a *data::resource_info*. The function creates a *data::resource* if the provided 
     * *data::resource_info rinfo* exists in the backend and is valid, and returns it 
     * coupled with a urd_error::success error code. Otherwise, return an appropriate error 
     * code and a std::shared_ptr<data::resource>{nullptr} */
    auto create_resource_from = [&](const resource_info_ptr rinfo) 
        -> std::tuple<urd_error, resource_ptr> {

        std::shared_ptr<storage::backend> backend_ptr = storage::remote_backend;
        const auto& nsid = rinfo->nsid();

        // all backends must be registered except for those
        // which are referenced by remote resources 
        if(!rinfo->is_remote()) {
            if(m_backends->count(nsid) == 0) {
                return std::make_tuple(urd_error::no_such_namespace, resource_ptr());
            }
            backend_ptr = m_backends->at(nsid);

            // check that resource is compatible with selected backend
            if(!backend_ptr->accepts(rinfo) || !backend_ptr->contains(rinfo)) {
                return std::make_tuple(urd_error::bad_args, resource_ptr());
            }
        }

        std::error_code ec;
        auto rsrc = data::make_resource(rinfo, backend_ptr, ec);

        if(ec) {
            LOGGER_ERROR("Could not create resource: {}", ec.message());
            return std::make_tuple(urd_error::bad_args, resource_ptr());
        }

        return std::make_tuple(urd_error::success, rsrc);
    };
#endif

    //XXX move to namespace-manager?
    auto get_backend = [&](const std::string& nsid, bool is_remote) 
        -> std::shared_ptr<storage::backend> {

        if(is_remote) {
            return storage::remote_backend;
        }

        const auto& it = m_backends->find(nsid);

        if(it != m_backends->end()) {
            return it->second;
        }

        return std::shared_ptr<storage::backend>();
    };

    const auto creds = request->credentials();

    if(!creds) {
        LOGGER_CRITICAL("Request without credentials");
        rv = urd_error::snafu; // TODO: invalid_credentials? eaccess? eperm?
        goto log_and_return;
    }

    //XXX move validate() to io::
    if((rv = validate_iotask_args(type, src_info, dst_info)) 
            == urd_error::success) {

        auto bsrc = get_backend(src_info->nsid(), src_info->is_remote());
        auto bdst = get_backend(dst_info->nsid(), dst_info->is_remote());

        if(bsrc == nullptr || bdst == nullptr) {
            rv = urd_error::no_such_namespace;
            goto log_and_return;
        }

        const auto tx_ptr = m_transferor_registry->get(src_info->type(), 
                                                       dst_info->type());

        if(!tx_ptr) {
            rv = urd_error::snafu; //TODO: unknown transferor
            goto log_and_return;
        }

        if(!tx_ptr->validate(src_info, dst_info)) {
            rv = urd_error::bad_args;
            goto log_and_return;
        }

        std::shared_ptr<io::task_stats> stats_record;

        // register the task in the task manager
        {
            boost::unique_lock<boost::shared_mutex> lock(m_task_manager_mutex);

            auto ret = m_task_manager->create();

            if(ret) {
                std::tie(tid, stats_record) = *ret;
            }
            else {
                // this can only happen if we tried to register a task
                // and the TID automatically generated collided with an
                // already running task. 
                // This can happen in two cases: 
                //   1. We end up with more than 4294967295U concurrent tasks
                //   2. We are not properly cleaning up dead tasks
                // In both cases, we want to know about it
                LOGGER_CRITICAL("Error when creating new task!");
                rv = urd_error::too_many_tasks;
            }
        }

        // everything is ok, add the I/O task to the queue
        if(stats_record != nullptr) {
            if(m_settings->m_dry_run) {
                m_workers->submit_and_forget(
                        io::fake_task(tid, stats_record));
            }
            else {
                m_workers->submit_and_forget(
                        io::task(tid, type, bsrc, src_info, bdst, dst_info, 
                                 *creds, std::move(tx_ptr), 
                                 std::move(stats_record)));
            }
        }

        rv = urd_error::success;
    }

log_and_return:
    resp = std::make_unique<api::iotask_create_response>(tid);
    resp->set_error_code(rv);

    LOGGER_INFO("IOTASK_CREATE({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::iotask_status_handler(const request_ptr base_request) const {

    auto resp = std::make_unique<api::iotask_status_response>();

    // downcast the generic request to the concrete implementation
    auto request = utils::static_unique_ptr_cast<api::iotask_status_request>(std::move(base_request));

    std::shared_ptr<io::task_stats> stats_ptr;

    {
        boost::shared_lock<boost::shared_mutex> lock(m_task_manager_mutex);
        stats_ptr = m_task_manager->find(request->get<0>());
    }


    if(stats_ptr != nullptr) {
        resp->set_error_code(urd_error::success);
        // *stats_ptr makes a copy of the current stats for this task in 
        // m_task_manager so that we don't block other threads needlessly 
        // while sending the reply
        resp->set<0>(io::task_stats_view(*stats_ptr));
    }
    else {
        resp->set_error_code(urd_error::no_such_task);
    }

    LOGGER_INFO("IOTASK_STATUS({}) = {}", request->to_string(), resp->to_string());

    return std::move(resp);
}


response_ptr urd::ping_handler(const request_ptr /*base_request*/) {
    response_ptr resp = std::make_unique<api::ping_response>();

    resp->set_error_code(urd_error::success);

    LOGGER_INFO("PING_REQUEST() = {}", resp->to_string());
    return resp;
}


response_ptr urd::job_register_handler(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::job_register_response>();

    // downcast the generic request to the concrete implementation
    auto request = utils::static_unique_ptr_cast<api::job_register_request>(std::move(base_request));

    uint32_t jobid = request->get<0>();
    auto hosts = request->get<1>();

    {
        boost::unique_lock<boost::shared_mutex> lock(m_jobs_mutex);

        if(m_jobs.find(jobid) != m_jobs.end()) {
            resp->set_error_code(urd_error::job_exists);
        }
        else {
            m_jobs.emplace(jobid, std::make_shared<job>(jobid, hosts));
            resp->set_error_code(urd_error::success);
        }
    }

    LOGGER_INFO("REGISTER_JOB({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::job_update_handler(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::job_update_response>();

    // downcast the generic request to the concrete implementation
    auto request = utils::static_unique_ptr_cast<api::job_update_request>(std::move(base_request));

    uint32_t jobid = request->get<0>();
    auto hosts = request->get<1>();

    {
        boost::unique_lock<boost::shared_mutex> lock(m_jobs_mutex);

        const auto& it = m_jobs.find(jobid);

        if(it == m_jobs.end()) {
            resp->set_error_code(urd_error::no_such_job);
        }
        else {
            it->second->update(hosts);
            resp->set_error_code(urd_error::success);
        }
    }

    LOGGER_INFO("UPDATE_JOB({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::job_remove_handler(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::job_unregister_response>();

    // downcast the generic request to the concrete implementation
    auto request = utils::static_unique_ptr_cast<api::job_unregister_request>(std::move(base_request));

    uint32_t jobid = request->get<0>();

    {
        boost::unique_lock<boost::shared_mutex> lock(m_jobs_mutex);

        const auto& it = m_jobs.find(jobid);

        if(it == m_jobs.end()) {
            resp->set_error_code(urd_error::no_such_job);
        }
        else {
            m_jobs.erase(it);
            resp->set_error_code(urd_error::success);
        }
    }

    LOGGER_INFO("UNREGISTER_JOB({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::process_add_handler(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::process_register_response>();

    // downcast the generic request to the concrete implementation
    auto request = utils::static_unique_ptr_cast<api::process_register_request>(std::move(base_request));

    uint32_t jobid = request->get<0>();
    pid_t uid = request->get<1>();
    gid_t gid = request->get<2>();
    pid_t pid = request->get<3>();

    boost::unique_lock<boost::shared_mutex> lock(m_jobs_mutex);

    const auto& it = m_jobs.find(jobid);

    if(it == m_jobs.end()) {
        resp->set_error_code(urd_error::no_such_job);
        goto log_and_return;
    }

    it->second->add_process(pid, gid);
    resp->set_error_code(urd_error::success);

log_and_return:
    LOGGER_INFO("ADD_PROCESS({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::process_remove_handler(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::process_unregister_response>();

    // downcast the generic request to the concrete implementation
    auto request = utils::static_unique_ptr_cast<api::process_unregister_request>(std::move(base_request));

    uint32_t jobid = request->get<0>();
    pid_t uid = request->get<1>();
    gid_t gid = request->get<2>();
    pid_t pid = request->get<3>();

    boost::unique_lock<boost::shared_mutex> lock(m_jobs_mutex);

    const auto& it = m_jobs.find(jobid);

    if(it == m_jobs.end()) {
        resp->set_error_code(urd_error::no_such_job);
        goto log_and_return;
    }

    if(it->second->find_and_remove_process(pid, gid)) {
        resp->set_error_code(urd_error::success);
    }
    else {
        resp->set_error_code(urd_error::no_such_process);
    }

log_and_return:
    LOGGER_INFO("REMOVE_PROCESS({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::backend_register_handler(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::backend_register_response>();

    // downcast the generic request to the concrete implementation
    auto request = utils::static_unique_ptr_cast<api::backend_register_request>(std::move(base_request));

    std::string nsid = request->get<0>();
    backend_type type = request->get<1>();
    std::string mount = request->get<2>();
    int32_t quota = request->get<3>();

    {
        boost::unique_lock<boost::shared_mutex> lock(m_backends_mutex);

        if(m_backends->count(nsid) != 0) {
            resp->set_error_code(urd_error::namespace_exists);
        }
        else {

            backend_ptr bptr = storage::backend_factory::create_from(type, mount, quota);

            if(bptr != nullptr) {
                m_backends->emplace(std::make_pair(nsid, bptr));
                resp->set_error_code(urd_error::success);
            }
            else {
                resp->set_error_code(urd_error::bad_args);
            }
        }
    }

    LOGGER_INFO("REGISTER_NAMESPACE({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

/* XXX not supported yet
response_ptr urd::backend_update_handler(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::backend_update_response>();

    // downcast the generic request to the concrete implementation
    auto request = utils::static_unique_ptr_cast<api::backend_update_request>(std::move(base_request));

    resp->set_error_code(urd_error::success);

log_and_return:
    LOGGER_INFO("UPDATE_NAMESPACE({}) = {}", request->to_string(), resp->to_string());
    return resp;
}*/

response_ptr urd::backend_remove_handler(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::backend_unregister_response>();

    // downcast the generic request to the concrete implementation
    auto request = utils::static_unique_ptr_cast<api::backend_unregister_request>(std::move(base_request));

    std::string nsid = request->get<0>();

    {
        boost::unique_lock<boost::shared_mutex> lock(m_backends_mutex);

        const auto& it = m_backends->find(nsid);

        if(it == m_backends->end()) {
            resp->set_error_code(urd_error::no_such_namespace);
        }
        else {
            m_backends->erase(it);
            resp->set_error_code(urd_error::success);
        }
    }

    LOGGER_INFO("UNREGISTER_NAMESPACE({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::unknown_request_handler(const request_ptr /*base_request*/) {
    response_ptr resp = std::make_unique<api::bad_request_response>();

    resp->set_error_code(urd_error::bad_request);

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

void urd::init_event_handlers() {

    LOGGER_INFO("  * Creating event listener...");

    // create (but not start) the API listener 
    // and register handlers for each request type
    m_api_listener = std::make_unique<api_listener>();

    boost::system::error_code ec;
    mode_t old_mask = umask(0);

    // if we find any socket files, but no pidfile was found,
    // they are stale sockets from another run. remove them.
    bfs::remove(m_settings->m_control_socket, ec);
    umask(S_IXUSR | S_IRWXG | S_IRWXO); // u=rw-, g=---, o=---
    m_api_listener->register_endpoint(m_settings->m_control_socket);

    bfs::remove(m_settings->m_global_socket, ec);
    umask(S_IXUSR | S_IXGRP | S_IXOTH); // u=rw-, g=rw-, o=rw-
    m_api_listener->register_endpoint(m_settings->m_global_socket);

    m_api_listener->register_endpoint(m_settings->m_remote_port);

    // restore the umask
    umask(old_mask);

    LOGGER_INFO("  * Installing message handlers...");

    /* user-level functionalities */
    m_api_listener->register_callback(
            api::request_type::iotask_create,
            std::bind(&urd::iotask_create_handler, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::iotask_status,
            std::bind(&urd::iotask_status_handler, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::ping,
            std::bind(&urd::ping_handler, this, std::placeholders::_1));

    /* admin-level functionalities */
    m_api_listener->register_callback(
            api::request_type::job_register,
            std::bind(&urd::job_register_handler, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::job_update,
            std::bind(&urd::job_update_handler, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::job_unregister,
            std::bind(&urd::job_remove_handler, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::process_register,
            std::bind(&urd::process_add_handler, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::process_unregister,
            std::bind(&urd::process_remove_handler, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::backend_register,
            std::bind(&urd::backend_register_handler, this, std::placeholders::_1));

/*    m_api_listener->register_callback(
            api::request_type::backend_update,
            std::bind(&urd::backend_update_handler, this, std::placeholders::_1));*/

    m_api_listener->register_callback(
            api::request_type::backend_unregister,
            std::bind(&urd::backend_remove_handler, this, std::placeholders::_1));

    m_api_listener->register_callback(
            api::request_type::bad_request,
            std::bind(&urd::unknown_request_handler, this, std::placeholders::_1));

    // signal handlers must be installed AFTER daemonizing
    LOGGER_INFO("  * Installing signal handlers...");

    m_api_listener->set_signal_handler(
        std::bind(&urd::signal_handler, this, std::placeholders::_1),
        SIGHUP, SIGTERM, SIGINT);
}

void urd::init_backend_descriptors() {

    // register POSIX filesystem backend
    storage::backend_factory::get().
        register_backend<storage::posix_filesystem>(backend_type::posix_filesystem,
            [](const bfs::path& mount, uint32_t quota) {
                return std::shared_ptr<storage::posix_filesystem>(
                        new storage::posix_filesystem(mount, quota));
            });

    // register NVML-DAX filesystem backend
    storage::backend_factory::get().
        register_backend<storage::nvml_dax>(backend_type::nvml,
            [](const bfs::path& mount, uint32_t quota) {
                return std::shared_ptr<storage::nvml_dax>(
                        new storage::nvml_dax(mount, quota));
            });

    // register Lustre backend
    storage::backend_factory::get().
        register_backend<storage::lustre>(backend_type::lustre,
            [](const bfs::path& mount, uint32_t quota) {
                return std::shared_ptr<storage::lustre>(
                        new storage::lustre(mount, quota));
            });
}

void urd::init_transferors() {

    LOGGER_INFO("  * Installing data transferors...");
    m_transferor_registry = std::make_unique<io::transferor_registry>();

    // memory region -> local path
    if(!m_transferor_registry->add(
        data::resource_type::memory_region, 
        data::resource_type::local_posix_path, 
        std::make_shared<io::memory_region_to_local_path_transferor>())) {

        LOGGER_ERROR("Could not register {} to {} transferor. Ignored.", 
                     utils::to_string(data::resource_type::memory_region),
                     utils::to_string(data::resource_type::local_posix_path));
    }

    // memory region -> shared path
    if(!m_transferor_registry->add(
        data::resource_type::memory_region, 
        data::resource_type::shared_posix_path, 
        std::make_shared<io::memory_region_to_local_path_transferor>())) {

        LOGGER_ERROR("Could not register {} to {} transferor. Ignored.", 
                     utils::to_string(data::resource_type::memory_region),
                     utils::to_string(data::resource_type::shared_posix_path));
    }

    // memory region -> remote path
    if(!m_transferor_registry->add(
        data::resource_type::memory_region, 
        data::resource_type::remote_posix_path, 
        std::make_shared<io::memory_region_to_local_path_transferor>())) {

        LOGGER_ERROR("Could not register {} to {} transferor. Ignored.", 
                     utils::to_string(data::resource_type::memory_region),
                     utils::to_string(data::resource_type::remote_posix_path));
    }

    // local path -> local path
    if(!m_transferor_registry->add(
        data::resource_type::local_posix_path, 
        data::resource_type::local_posix_path, 
        std::make_shared<io::local_path_to_local_path_transferor>())) {

        LOGGER_ERROR("Could not register {} to {} transferor. Ignored.", 
                     utils::to_string(data::resource_type::local_posix_path),
                     utils::to_string(data::resource_type::local_posix_path));
    }

    // local path -> shared path
    if(!m_transferor_registry->add(
        data::resource_type::local_posix_path, 
        data::resource_type::shared_posix_path, 
        std::make_shared<io::local_path_to_shared_path_transferor>())) {

        LOGGER_ERROR("Could not register {} to {} transferor. Ignored.", 
                     utils::to_string(data::resource_type::local_posix_path),
                     utils::to_string(data::resource_type::shared_posix_path));
    }

    // local path -> remote path
    if(!m_transferor_registry->add(
        data::resource_type::local_posix_path, 
        data::resource_type::remote_posix_path, 
        std::make_shared<io::local_path_to_remote_path_transferor>())) {

        LOGGER_ERROR("Could not register {} to {} transferor. Ignored.", 
                     utils::to_string(data::resource_type::local_posix_path),
                     utils::to_string(data::resource_type::remote_posix_path));
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
    LOGGER_INFO("[[ Settings ]]");
    LOGGER_INFO("  - running as daemon?: {}", (m_settings->m_daemonize ? "yes" : "no"));
    LOGGER_INFO("  - dry run?: {}", (m_settings->m_dry_run ? "yes" : "no"));
    LOGGER_INFO("  - pidfile: {}", m_settings->m_daemon_pidfile);
    LOGGER_INFO("  - control socket: {}", m_settings->m_control_socket);
    LOGGER_INFO("  - global socket: {}", m_settings->m_global_socket);
    LOGGER_INFO("  - port for remote requests: {}", m_settings->m_remote_port);
    LOGGER_INFO("  - workers: {}", m_settings->m_workers_in_pool);
    LOGGER_INFO("");
//    LOGGER_INFO("    - internal storage: {}", m_settings->m_storage_path);
//    LOGGER_INFO("    - internal storage capacity: {}", m_settings->m_storage_capacity);

    LOGGER_INFO("[[ Starting up ]]");

    // create worker pool
    LOGGER_INFO("  * Creating workers...");
    m_workers = std::make_unique<thread_pool>(m_settings->m_workers_in_pool);

    init_event_handlers();

    m_backends = std::make_unique<backend_manager>();

    // pre-register special backends
    //TODO define constants
    m_backends->emplace("[[internal::memory]]", storage::process_memory_backend);

    m_task_manager = std::make_unique<io::task_manager>();

    init_backend_descriptors();

    init_transferors();


    LOGGER_INFO("");
    LOGGER_INFO("[[ Start up successful, awaiting requests... ]]");
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

//XXX deprecated, signals_are now managed by api_listener
//    if(m_signal_listener) {
//        LOGGER_INFO("* Stopping signal listener...");
//        m_signal_listener->stop();
//        //m_signal_listener.reset();
//    }

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

} // namespace norns
