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

#ifdef __LOGGER_ENABLE_DEBUG__
#include <sys/prctl.h>
#endif

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/atomic.hpp>
#include <functional>

#include "common.hpp"
#include "api.hpp"
#include "backends.hpp"
#include "logger.hpp"
#include "job.hpp"
#include "resources.hpp"
#include "io.hpp"
#include "namespaces.hpp"
#include "fmt.hpp"
#include "hermes.hpp"
#include "rpcs.hpp"
#include "context.hpp"
#include "urd.hpp"

namespace norns {

urd::urd() :
    m_is_paused(false),
    m_settings(std::make_shared<config::settings>()) {}

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
        LOGGER_ERRNO("Failed to create child process");
        exit(EXIT_FAILURE);
    }

    /* Parent returns to caller */
    if(pid != 0) {
        return pid;
    }

    /* Become a session and process group leader with no controlling tty */
    if((sid = setsid()) < 0) {
        /* Log failure */
        LOGGER_ERRNO("Failed to disassociate controlling tty");
        exit(EXIT_FAILURE);
    }

    /* Handle standard IO: discard data to/from stdin, stdout and stderr */
    int dev_null;

    if((dev_null = open("/dev/null", O_RDWR)) == -1) {
        LOGGER_ERRNO("Failed to open \"/dev/null\"");
        exit(EXIT_FAILURE);
    }

    if(dup2(dev_null, STDIN_FILENO) == -1) {
        LOGGER_ERRNO("Failed to dup \"/dev/null\" onto stdin");
        exit(EXIT_FAILURE);
    }

    if(dup2(dev_null, STDOUT_FILENO) == -1) {
        LOGGER_ERRNO("Failed to dup \"/dev/null\" onto stdout");
        exit(EXIT_FAILURE);
    }

    if(dup2(dev_null, STDERR_FILENO) == -1) {
        LOGGER_ERRNO("Failed to dup \"/dev/null\" onto stderr");
        exit(EXIT_FAILURE);
    }

    /* Change the file mode creation mask */
    umask(0);

    /* ensure the process does not keep a directory in use,
     * avoid relative paths beyond this point! */
    if(chdir("/") < 0) {
        LOGGER_ERRNO("Failed to change working directory to root directory");
        exit(EXIT_FAILURE);
    }
    
    /* Check if daemon already exists:
     * First instance of the daemon will lock the file so that other
     * instances understand that an instance is already running. 
     */
    int pfd;
    
    if((pfd = open(m_settings->pidfile().c_str(), O_RDWR | O_CREAT, 0640)) == -1) {
        LOGGER_ERRNO("Failed to create daemon lock file");
        exit(EXIT_FAILURE);
    } 

    if(lockf(pfd, F_TLOCK, 0) < 0) {
        LOGGER_ERRNO("Failed to acquire lock on pidfile");
        LOGGER_ERROR("Another instance of this daemon may already be running");
        exit(EXIT_FAILURE);
    }

    /* record pid in lockfile */
    std::string pidstr(std::to_string(getpid()));

    if(write(pfd, pidstr.c_str(), pidstr.length()) != 
            static_cast<ssize_t>(pidstr.length())) {
        LOGGER_ERRNO("Failed to write pidfile");
        exit(EXIT_FAILURE);
    }

    close(pfd);
    close(dev_null);

    /* Manage signals */
    signal(SIGCHLD, SIG_IGN); /* ignore child */
    signal(SIGTSTP, SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
//  signal(SIGHUP, signal_handler); /* catch hangup signal */
//  signal(SIGTERM, signal_handler); /* catch kill signal */

    return 0;
}

urd_error urd::validate_iotask_args(iotask_type type, 
                                    const resource_info_ptr& src_rinfo,
                                    const resource_info_ptr& dst_rinfo) const {

    if(type != iotask_type::copy && 
       type != iotask_type::move &&
       type != iotask_type::remove) {
        return urd_error::bad_args;
    }

    if(src_rinfo->is_remote() && dst_rinfo->is_remote()) {
        return urd_error::not_supported;
    }

//    // dst_resource cannot be a memory region
//    if(dst_rinfo->type() == data::resource_type::memory_region) {
//        return urd_error::not_supported;
//    }

    return urd_error::success;
}


///////////////////////////////////////////////////////////////////////////////
//                 handlers for user requests
///////////////////////////////////////////////////////////////////////////////

response_ptr 
urd::iotask_create_handler(const request_ptr base_request) {

    // downcast the generic request to the concrete implementation
    auto request = 
        utils::static_unique_ptr_cast<api::iotask_create_request>(
                std::move(base_request));

    const auto type = request->get<0>();
    const auto src_rinfo = request->get<1>();
    const auto dst_rinfo = request->get<2>().get_value_or(nullptr);

    std::vector<std::string> nsids;
    std::vector<bool> remotes;
    std::vector<std::shared_ptr<storage::backend>> backend_ptrs;
    std::vector<std::shared_ptr<data::resource_info>> rinfo_ptrs;
    boost::optional<iotask_id> tid;
    boost::optional<auth::credentials> auth;
    boost::optional<io::generic_task> t;
    response_ptr resp;
    urd_error rv = urd_error::success;

    if(m_is_paused) {
        rv = urd_error::accept_paused;
        goto log_and_return;
    }

    auth = request->credentials();

    if(!auth) {
        LOGGER_CRITICAL("Request without credentials");
        rv = urd_error::snafu; // TODO: invalid_credentials? eaccess? eperm?
        goto log_and_return;
    }

//    if(src_rinfo->is_remote()) {
//        rv = urd_error::not_supported;
//        goto log_and_return;
//    }

    for(const auto& rinfo : {src_rinfo, dst_rinfo}) {
        if(rinfo) {
            nsids.push_back(rinfo->nsid());
            remotes.push_back(rinfo->is_remote());
            rinfo_ptrs.emplace_back(rinfo);
        }
    }

#ifdef __LOGGER_ENABLE_DEBUG__
    LOGGER_DEBUG("Request metadata:");
    LOGGER_DEBUG("  rinfos: {} {}", src_rinfo, dst_rinfo);

    LOGGER_DEBUG("  nsids: {");
    for(std::size_t i=0; i<nsids.size(); ++i) {
        LOGGER_DEBUG("    \"{}\"", nsids[i]);
    }
    LOGGER_DEBUG("  }");

    LOGGER_DEBUG("  remotes: {");
    for(std::size_t i=0; i<remotes.size(); ++i) {
        LOGGER_DEBUG("    {}", remotes[i]);
    }
    LOGGER_DEBUG("  }");
#endif

    {
        bool all_found = false;
        boost::shared_lock<boost::shared_mutex> lock(m_namespace_mgr_mutex);
        std::tie(all_found, backend_ptrs) = m_namespace_mgr->find(nsids, remotes);

        if(!all_found) {
            rv = urd_error::no_such_namespace;
            goto log_and_return;
        }
    }

#ifdef __LOGGER_ENABLE_DEBUG__
    for(std::size_t i=0; i<nsids.size(); ++i) {
        LOGGER_DEBUG("nsid: {}, remote?: {}, bptr: {}", nsids[i], remotes[i], backend_ptrs[i]);
    }
#endif


    //FIXME: use appropriate args for each task rather than a vector of nullptrs
    switch(type) {
        case iotask_type::move:
        case iotask_type::copy:
            std::tie(rv, t) = 
                m_task_mgr->create_local_initiated_task(type, *auth, backend_ptrs, rinfo_ptrs);
            break;
        case iotask_type::remove:
            std::tie(rv, t) =
                m_task_mgr->create_local_initiated_task(type, *auth, backend_ptrs, rinfo_ptrs);
            break;
        case iotask_type::noop:
            std::tie(rv, t) = 
                m_task_mgr->create_local_initiated_task(type, *auth, backend_ptrs, rinfo_ptrs);
            break;
        default:
            rv = urd_error::bad_args;
            goto log_and_return;
    }

    if(rv == urd_error::success) {
        tid = t->id();

        // enqueue task so that it's eventually run by a worker thread
        m_task_mgr->enqueue_task(std::move(*t));
    }

//    std::tie(rv, tid) = m_task_mgr->create_task(type, *auth, backend_ptrs, rinfo_ptrs);

log_and_return:
    resp = std::make_unique<api::iotask_create_response>(tid.get_value_or(0));
    resp->set_error_code(rv);

    LOGGER_INFO("IOTASK_CREATE({}) = {}", request->to_string(), resp->to_string());
    return resp;
}


response_ptr 
urd::iotask_status_handler(const request_ptr base_request) const {

    auto resp = std::make_unique<api::iotask_status_response>();

    // downcast the generic request to the concrete implementation
    auto request = 
        utils::static_unique_ptr_cast<api::iotask_status_request>(
                std::move(base_request));

    auto task_info_ptr = m_task_mgr->find(request->get<0>());

    if(task_info_ptr) {
        resp->set_error_code(urd_error::success);

        // stats provides a thread-safe view of a task status 
        // (locking is done internally)
        resp->set<0>(task_info_ptr->stats());

        if(task_info_ptr->status() == io::task_status::finished ||
           task_info_ptr->status() == io::task_status::finished_with_error) {
            m_task_mgr->erase(request->get<0>());
        }

    }
    else {
        resp->set_error_code(urd_error::no_such_task);
    }

    LOGGER_INFO("IOTASK_STATUS({}) = {}", 
                request->to_string(), resp->to_string());

    return std::move(resp);
}


///////////////////////////////////////////////////////////////////////////////
//                 handlers for control requests
///////////////////////////////////////////////////////////////////////////////

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
    //pid_t uid = request->get<1>();
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
    //pid_t uid = request->get<1>();
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

urd_error urd::create_namespace(const config::namespace_def& nsdef) {

    const auto type = storage::backend_factory::get().
        get_type(nsdef.alias());

    if(type == backend_type::unknown) {
        return urd_error::bad_args;
    }

    return create_namespace(nsdef.nsid(), type, nsdef.track(), 
                            nsdef.mountpoint(), nsdef.capacity());
}

urd_error urd::create_namespace(const std::string& nsid, backend_type type,
                                bool track, const bfs::path& mount, 
                                uint32_t quota) {

    boost::unique_lock<boost::shared_mutex> lock(m_namespace_mgr_mutex);

    if(m_namespace_mgr->contains(nsid)) {
        return urd_error::namespace_exists;
    }

    if(auto bptr = storage::backend_factory::create_from(
                type, nsid, track, mount, quota)) {
        m_namespace_mgr->add(nsid, bptr);
        return urd_error::success;
    }

    return urd_error::bad_args;
}

response_ptr urd::namespace_register_handler(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::backend_register_response>();

    // downcast the generic request to the concrete implementation
    auto request = utils::static_unique_ptr_cast<api::backend_register_request>(std::move(base_request));

    std::string nsid = request->get<0>();
    backend_type type = request->get<1>();
    bool track = request->get<2>();
    std::string mount = request->get<3>();
    int32_t quota = request->get<4>();

    resp->set_error_code(create_namespace(nsid, type, track, mount, quota));

    LOGGER_INFO("REGISTER_NAMESPACE({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

/* XXX not supported yet
response_ptr urd::namespace_update_handler(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::backend_update_response>();

    // downcast the generic request to the concrete implementation
    auto request = utils::static_unique_ptr_cast<api::backend_update_request>(std::move(base_request));

    resp->set_error_code(urd_error::success);

log_and_return:
    LOGGER_INFO("UPDATE_NAMESPACE({}) = {}", request->to_string(), resp->to_string());
    return resp;
}*/

response_ptr urd::namespace_remove_handler(const request_ptr base_request) {

    response_ptr resp = std::make_unique<api::backend_unregister_response>();

    // downcast the generic request to the concrete implementation
    auto request = utils::static_unique_ptr_cast<api::backend_unregister_request>(std::move(base_request));

    std::string nsid = request->get<0>();

    {
        boost::unique_lock<boost::shared_mutex> lock(m_namespace_mgr_mutex);

        if(m_namespace_mgr->remove(nsid)) {
            resp->set_error_code(urd_error::success);
        }
        else {
            resp->set_error_code(urd_error::no_such_namespace);
        }
    }

    LOGGER_INFO("UNREGISTER_NAMESPACE({}) = {}", request->to_string(), resp->to_string());
    return resp;
}

response_ptr urd::global_status_handler(const request_ptr /*base_request*/) {

    auto resp = std::make_unique<api::global_status_response>();

    resp->set_error_code(urd_error::success);
    resp->set<0>(m_task_mgr->global_stats());

    LOGGER_INFO("GLOBAL_STATUS() = {}", resp->to_string());
    return std::move(resp);
}

response_ptr
urd::command_handler(const request_ptr base_request) {

    // downcast the generic request to the concrete implementation
    auto request = 
        utils::static_unique_ptr_cast<api::command_request>(
                std::move(base_request));
    auto resp = std::make_unique<api::command_response>();
    resp->set_error_code(urd_error::success);

    switch(request->get<0>()) {
        case command_type::ping:
            break; // nothing special to do here
        case command_type::pause_listen:
            pause_listening();
            break;
        case command_type::resume_listen:
            resume_listening();
            break;
        case command_type::shutdown:
        {
            LOGGER_WARN("Shutdown requested!");
            pause_listening();

            const auto rv = check_shutdown();
            resp->set_error_code(rv);

            if(rv != urd_error::success) {
                resume_listening();
                break;
            }
            shutdown();
            break;
        }
        case command_type::unknown:
            resp->set_error_code(urd_error::bad_args);
            break;
    }

    LOGGER_INFO("COMMAND({}) = {}", request->to_string(), resp->to_string());
    return std::move(resp);
}


response_ptr urd::unknown_request_handler(const request_ptr /*base_request*/) {
    response_ptr resp = std::make_unique<api::bad_request_response>();

    resp->set_error_code(urd_error::bad_request);

    LOGGER_INFO("UNKNOWN_REQUEST() = {}", resp->to_string());
    return resp;
}

// N.B. This function is called by the progress thread internal to 
// m_network_service rather than by the main execution thread
void
urd::push_resource_handler(hermes::request<rpc::push_resource>&& req) {

    const auto args = req.args();

    LOGGER_WARN("incoming rpc::push_resource(from: \"{}@{}:{}\", to: \"{}:{}\")", 
                args.in_nsid(), 
                args.in_address(),
                args.in_resource_name(),
                args.out_nsid(),
                args.out_resource_name());

    urd_error rv = urd_error::success;
    boost::optional<io::generic_task> t;
    std::shared_ptr<storage::backend> src_backend;
    boost::optional<std::shared_ptr<storage::backend>> dst_backend;
    auto dst_rtype = static_cast<data::resource_type>(args.out_resource_type());
    auth::credentials auth; //XXX fake credentials for now

    const auto create_rinfo = 
        [&](const data::resource_type& rtype) -> 
            std::shared_ptr<data::resource_info> {

        switch(rtype) {
            case data::resource_type::remote_resource:
                return std::make_shared<data::remote_resource_info>(
                        args.in_address(), args.in_nsid(), 
                        args.in_is_collection(), args.in_resource_name(), 
                        args.in_buffers());
            case data::resource_type::local_posix_path:
            case data::resource_type::shared_posix_path:
                return std::make_shared<data::local_path_info>(
                        args.out_nsid(), args.out_resource_name());
            default:
                rv = urd_error::not_supported;
                return {};
        }
    };

    if(m_is_paused) {
        rv = urd_error::accept_paused;
        LOGGER_INFO("IOTASK_RECEIVE() = {}", utils::to_string(rv));
        m_network_service->respond(std::move(req), 
                    static_cast<uint32_t>(io::task_status::finished_with_error),
                    static_cast<uint32_t>(rv),
                    0,
                    0);
        return;
    }

    auto src_rinfo = create_rinfo(data::resource_type::remote_resource);
    auto dst_rinfo = create_rinfo(dst_rtype);

    //TODO: check src_rinfo and dst_rinfo


    // TODO: actually retrieve and validate credentials, etc


    src_backend = 
        std::make_shared<storage::detail::remote_backend>(args.in_nsid());

    {
        boost::shared_lock<boost::shared_mutex> lock(m_namespace_mgr_mutex);
        dst_backend = m_namespace_mgr->find(args.out_nsid());
    }

    if(!dst_backend) {
        rv = urd_error::no_such_namespace;
        LOGGER_INFO("IOTASK_RECEIVE() = {}", utils::to_string(rv));
        m_network_service->respond(std::move(req), 
                static_cast<uint32_t>(io::task_status::finished_with_error),
                static_cast<int32_t>(rv),
                0,
                0);
        return;
    }

    LOGGER_DEBUG("nsid: {}, bptr: {}", args.in_nsid(), dst_backend);

    auto ctx =
        std::make_shared<hermes::request<rpc::push_resource>>(std::move(req));

    std::tie(rv, t) =
        m_task_mgr->create_remote_initiated_task(
                iotask_type::remote_transfer, auth, 
                ctx, src_backend, src_rinfo, 
                *dst_backend, dst_rinfo);

    if(rv == urd_error::success) {
        // run the task and check that it started correctly
        (*t)();

        auto req = std::move(*ctx);

        if(t->info()->status() == io::task_status::finished_with_error) {
            rv = t->info()->task_error();
            LOGGER_INFO("IOTASK_RECEIVE() = {}", utils::to_string(rv));
            m_network_service->respond(std::move(req), 
                    static_cast<uint32_t>(io::task_status::finished_with_error),
                    static_cast<int32_t>(t->info()->task_error()),
                    static_cast<int32_t>(t->info()->sys_error().value()),
                    0);
        }
    }
}

void
urd::pull_resource_handler(hermes::request<rpc::pull_resource>&& req) {

    const auto args = req.args();

    LOGGER_WARN("incoming rpc::push_request(from: \"{}:{}\", to: \"{}@{}:{}\")", 
                args.in_nsid(), 
                args.in_resource_name(),
                args.out_address(),
                args.out_nsid(), 
                args.out_resource_name());

    urd_error rv = urd_error::success;
    boost::optional<io::generic_task> tsk;
    boost::optional<std::shared_ptr<storage::backend>> src_backend;
    std::shared_ptr<storage::backend> dst_backend;

    /// XXX this information should be retrievable from the backend
    auto src_rtype = static_cast<data::resource_type>(args.in_resource_type());

    auth::credentials auth; //XXX fake credentials for now

    const auto create_rinfo = 
        [&](const data::resource_type& rtype) -> 
            std::shared_ptr<data::resource_info> {

        switch(rtype) {
            case data::resource_type::remote_resource:
                return std::make_shared<data::remote_resource_info>(
                        args.out_address(), args.out_nsid(), false, 
                        args.out_resource_name(), args.out_buffers());
            case data::resource_type::local_posix_path:
            case data::resource_type::shared_posix_path:
                return std::make_shared<data::local_path_info>(
                        args.in_nsid(), args.in_resource_name());
            default:
                rv = urd_error::not_supported;
                return {};
        }
    };

    if(m_is_paused) {
        rv = urd_error::accept_paused;
        LOGGER_INFO("IOTASK_RECEIVE() = {}", utils::to_string(rv));
        m_network_service->respond(std::move(req), 
                    static_cast<uint32_t>(io::task_status::finished_with_error),
                    static_cast<uint32_t>(rv),
                    0,
                    0);
        return;
    }

    auto src_rinfo = create_rinfo(src_rtype);
    auto dst_rinfo = create_rinfo(data::resource_type::remote_resource);

    //TODO: check src_rinfo and dst_rinfo


    // TODO: actually retrieve and validate credentials, etc

    {
        boost::shared_lock<boost::shared_mutex> lock(m_namespace_mgr_mutex);
        src_backend = m_namespace_mgr->find(args.in_nsid());
    }

    if(!src_backend) {
        rv = urd_error::no_such_namespace;
        LOGGER_INFO("IOTASK_RECEIVE() = {}", utils::to_string(rv));
        m_network_service->respond(std::move(req), 
                static_cast<uint32_t>(io::task_status::finished_with_error),
                static_cast<int32_t>(rv),
                0,
                0);
        return;
    }

    LOGGER_DEBUG("nsid: {}, bptr: {}", args.in_nsid(), src_backend);

    dst_backend = 
        std::make_shared<storage::detail::remote_backend>(args.out_nsid());

    auto ctx =
        std::make_shared<hermes::request<rpc::pull_resource>>(std::move(req));

    std::tie(rv, tsk) =
        m_task_mgr->create_remote_initiated_task(
                iotask_type::remote_transfer, auth, 
                ctx, *src_backend, src_rinfo, 
                dst_backend, dst_rinfo);

    if(rv == urd_error::success) {
        // run the task and check that it started correctly
        (*tsk)();

        auto req = std::move(*ctx);

        if(tsk->info()->status() == io::task_status::finished_with_error) {
            rv = tsk->info()->task_error();
            LOGGER_INFO("IOTASK_RECEIVE() = {}", utils::to_string(rv));
            m_network_service->respond(std::move(req), 
                    static_cast<uint32_t>(io::task_status::finished_with_error),
                    static_cast<int32_t>(tsk->info()->task_error()),
                    static_cast<int32_t>(tsk->info()->sys_error().value()),
                    0);
        }
    }
}

void
urd::stat_resource_handler(hermes::request<rpc::stat_resource>&& req) {

    const auto args = req.args();

    LOGGER_WARN("incoming rpc::stat_resource(\"{}:{}\")", 
                args.nsid(), 
                args.resource_name());

    urd_error rv = urd_error::success;
    boost::optional<std::shared_ptr<storage::backend>> dst_backend;

    {
        boost::shared_lock<boost::shared_mutex> lock(m_namespace_mgr_mutex);
        dst_backend = m_namespace_mgr->find(args.nsid());
    }

    if(!dst_backend) {
        rv = urd_error::no_such_namespace;
        LOGGER_INFO("IOTASK_RECEIVE() = {}", utils::to_string(rv));
        m_network_service->respond(std::move(req), 
                                    static_cast<uint32_t>(rv),
                                    false,
                                    //XXX ENOENT should not be required:
                                    // the transfer() interface should be
                                    // richer rather than returning only a
                                    // std::error_code
                                    ENOENT,
                                    0);
        return;
    }

    auto rtype = static_cast<data::resource_type>(args.resource_type());

    const auto create_rinfo = 
        [&](const data::resource_type& rtype) -> 
            std::shared_ptr<data::resource_info> {

        switch(rtype) {
            case data::resource_type::local_posix_path:
            case data::resource_type::shared_posix_path:
                return std::make_shared<data::local_path_info>(
                        args.nsid(), args.resource_name());
            default:
                return {};
        }
    };

    auto rinfo = create_rinfo(rtype);

    if(!rinfo) {
        LOGGER_ERROR("Failed to access resource {}", rinfo->to_string());
        rv = urd_error::not_supported;

        LOGGER_INFO("IOTASK_RECEIVE() = {}", utils::to_string(rv));
        m_network_service->respond(std::move(req), 
                                    static_cast<uint32_t>(rv),
                                    //XXX EOPNOTSUPP should not be required:
                                    // the transfer() interface should be
                                    // richer rather than returning only a
                                    // std::error_code
                                    EOPNOTSUPP, 
                                    false,
                                    0);
        return;
    }

    std::error_code ec;
    auto rsrc = (*dst_backend)->get_resource(rinfo, ec);

    if(ec) {
        LOGGER_ERROR("Failed to access resource {}", rinfo->to_string());
        rv = urd_error::no_such_resource;

        LOGGER_INFO("IOTASK_RECEIVE() = {}", utils::to_string(rv));
        m_network_service->respond(std::move(req), 
                                    static_cast<uint32_t>(rv),
                                    ec.value(),
                                    false,
                                    0);
        return;
    }

    LOGGER_INFO("IOTASK_RECEIVE() = {}", utils::to_string(rv));
    m_network_service->respond(std::move(req), 
            static_cast<uint32_t>(urd_error::success),
            0,
            rsrc->is_collection(),
            rsrc->packed_size());
}


void urd::configure(const config::settings& settings) {
    m_settings = std::make_shared<config::settings>(settings);
}

config::settings urd::get_configuration() const {
    return *m_settings;
}

void urd::signal_handler(int signum){

    switch(signum) {

        case SIGINT:
            LOGGER_WARN("A signal (SIGINT) occurred.");
            shutdown();
            break;

        case SIGTERM:
            LOGGER_WARN("A signal (SIGTERM) occurred.");
            shutdown();
            break;

        case SIGHUP:
            LOGGER_WARN("A signal (SIGHUP) occurred.");
            break;
    }
}

void urd::init_logger() {

    if(m_settings->use_console()) {
        logger::create_global_logger(m_settings->progname(), "console color");
        return;;
    }

    if(m_settings->use_syslog()) {
        logger::create_global_logger(m_settings->progname(), "syslog");

        if(!m_settings->daemonize()) {
            fmt::print(stderr, "PSA: Output sent to syslog while in "
                               "non-daemon mode\n");
        }

        return;
    }

    if(!m_settings->log_file().empty()) {
        logger::create_global_logger(m_settings->progname(), "file", 
                m_settings->log_file());
        return;
    }

    logger::create_global_logger(m_settings->progname(), "console color");
}

void urd::init_event_handlers() {

    LOGGER_INFO(" * Creating event listener...");

    // create (but not start) the API listener 
    // and register handlers for each request type
    try {
        m_ipc_service = std::make_unique<api_listener>();
    }
    catch(const std::exception& e) {
        LOGGER_ERROR("Failed to create the event listener. This should "
                     "not happen under normal conditions.");
        exit(EXIT_FAILURE);
    }

    boost::system::error_code ec;
    mode_t old_mask = ::umask(0);

    // setup socket for control API
    // (note that if we find any socket files at this point, where no pidfile 
    // was found during initialization, they must be stale sockets from 
    // another run. Just remove them).
    if(bfs::exists(m_settings->control_socket())) {
        bfs::remove(m_settings->control_socket(), ec);

        if(ec) {
            LOGGER_ERROR("Failed to remove stale control API socket: {}", 
                    ec.message());
            teardown();
            exit(EXIT_FAILURE);
        }
    }

    ::umask(S_IXUSR | S_IRWXG | S_IRWXO); // u=rw-, g=---, o=---

    try {
        m_ipc_service->register_endpoint(m_settings->control_socket());
    }
    catch(const std::exception& e) {
        LOGGER_ERROR("Failed to create control API socket: {}", e.what());
        teardown();
        exit(EXIT_FAILURE);
    }

    // setup socket for user API
    if(bfs::exists(m_settings->global_socket())) {
        bfs::remove(m_settings->global_socket(), ec);

        if(ec) {
            LOGGER_ERROR("Failed to remove stale user API socket: {}", 
                    ec.message());
            teardown();
            exit(EXIT_FAILURE);
        }
    }

    ::umask(S_IXUSR | S_IXGRP | S_IXOTH); // u=rw-, g=rw-, o=rw-

    try {
        m_ipc_service->register_endpoint(m_settings->global_socket());
    }
    catch(const std::exception& e) {
        LOGGER_ERROR("Failed to create user API socket: {}", e.what());
        teardown();
        exit(EXIT_FAILURE);
    }

    // restore the umask
    ::umask(old_mask);

    try {

        const std::string bind_address = 
            m_settings->bind_address() + ":" +
            std::to_string(m_settings->remote_port());

        m_network_service = 
            std::make_shared<hermes::async_engine>(
                    hermes::transport::ofi_tcp,
                    bind_address,
                    true);
    }
    catch(const std::exception& e) {
        LOGGER_ERROR("Failed to create remote listener: {}", e.what());
        teardown();
        exit(EXIT_FAILURE);
    }

#if 0
    // setup socket for remote connections
    try {
        m_ipc_service->register_endpoint(m_settings->remote_port());
    }
    catch(const std::exception& e) {
        LOGGER_ERROR("Failed to create socket for remote connections: {}",
                e.what());
        teardown();
        exit(EXIT_FAILURE);
    }
#endif


    LOGGER_INFO(" * Installing message handlers...");

    /* user-level functionalities */
    m_ipc_service->register_callback(
            api::request_type::iotask_create,
            std::bind(&urd::iotask_create_handler, this, std::placeholders::_1));

    m_ipc_service->register_callback(
            api::request_type::iotask_status,
            std::bind(&urd::iotask_status_handler, this, std::placeholders::_1));

    m_ipc_service->register_callback(
            api::request_type::ping,
            std::bind(&urd::ping_handler, this, std::placeholders::_1));

    /* admin-level functionalities */
    m_ipc_service->register_callback(
            api::request_type::job_register,
            std::bind(&urd::job_register_handler, this, std::placeholders::_1));

    m_ipc_service->register_callback(
            api::request_type::job_update,
            std::bind(&urd::job_update_handler, this, std::placeholders::_1));

    m_ipc_service->register_callback(
            api::request_type::job_unregister,
            std::bind(&urd::job_remove_handler, this, std::placeholders::_1));

    m_ipc_service->register_callback(
            api::request_type::process_register,
            std::bind(&urd::process_add_handler, this, std::placeholders::_1));

    m_ipc_service->register_callback(
            api::request_type::process_unregister,
            std::bind(&urd::process_remove_handler, this, std::placeholders::_1));

    m_ipc_service->register_callback(
        api::request_type::backend_register,
        std::bind(&urd::namespace_register_handler, this,
                  std::placeholders::_1));

    /*    m_ipc_service->register_callback(
                api::request_type::backend_update,
                std::bind(&urd::namespace_update_handler, this,
       std::placeholders::_1));*/

    m_ipc_service->register_callback(
            api::request_type::backend_unregister,
            std::bind(&urd::namespace_remove_handler, this, std::placeholders::_1));

    m_ipc_service->register_callback(
            api::request_type::global_status,
            std::bind(&urd::global_status_handler, this, std::placeholders::_1));

    m_ipc_service->register_callback(
            api::request_type::command,
            std::bind(&urd::command_handler, this, std::placeholders::_1));

    m_ipc_service->register_callback(
            api::request_type::bad_request,
            std::bind(&urd::unknown_request_handler, this, std::placeholders::_1));

    /* remote event handlers */
    m_network_service->register_handler<rpc::push_resource>(
            std::bind(&urd::push_resource_handler, this, 
                      std::placeholders::_1));

    m_network_service->register_handler<rpc::pull_resource>(
            std::bind(&urd::pull_resource_handler, this, 
                      std::placeholders::_1));

    m_network_service->register_handler<rpc::stat_resource>(
            std::bind(&urd::stat_resource_handler, this, 
                      std::placeholders::_1));


    // signal handlers must be installed AFTER daemonizing
    LOGGER_INFO(" * Installing signal handlers...");

    m_ipc_service->set_signal_handler(
        std::bind(&urd::signal_handler, this, std::placeholders::_1),
        SIGHUP, SIGTERM, SIGINT);
}

void urd::init_namespace_manager() {

    LOGGER_INFO(" * Creating namespace manager...");

    try {
        m_namespace_mgr = std::make_unique<ns::namespace_manager>();
    }
    catch(const std::exception& e) {
        LOGGER_ERROR("Failed to create the namespace manager. This should "
                     "not happen under normal conditions.");
        exit(EXIT_FAILURE);
    }

    // pre-register special backends
    //TODO define constants
    if(!m_namespace_mgr->add("[[internal::memory]]", 
                             storage::process_memory_backend)) {
        LOGGER_ERROR("Failed to register internal memory namespace");
        exit(EXIT_FAILURE);
    }
}

void urd::init_task_manager() {

    LOGGER_INFO(" * Creating task manager...");

    try {
        m_task_mgr = std::make_unique<io::task_manager>(m_settings->workers_in_pool(),
                                                        m_settings->backlog_size(),
                                                        m_settings->dry_run(),
                                                        m_settings->dry_run_duration());
    }
    catch(const std::exception& e) {
        LOGGER_ERROR("Failed to create the task manager. This should "
                     "not happen under normal conditions.");
        exit(EXIT_FAILURE);
    }
}

void urd::load_backend_plugins() {

    // register POSIX filesystem backend plugin
    storage::backend_factory::get().
        register_backend<storage::posix_filesystem>(
            backend_type::posix_filesystem,
            [](const std::string& nsid, bool track, 
               const bfs::path& mount, uint32_t quota) {
                return std::shared_ptr<storage::posix_filesystem>(
                        new storage::posix_filesystem(nsid, track, mount, quota));
            });

    storage::backend_factory::get().
        register_alias("POSIX/LOCAL", backend_type::posix_filesystem);
    storage::backend_factory::get().
        register_alias("POSIX/SHARED", backend_type::posix_filesystem);

    // register NVML-DAX filesystem backend plugin
    storage::backend_factory::get().
        register_backend<storage::nvml_dax>(
            backend_type::nvml,
            [](const std::string& nsid, bool track, 
               const bfs::path& mount, uint32_t quota) {
                return std::shared_ptr<storage::nvml_dax>(
                        new storage::nvml_dax(nsid, track, mount, quota));
            });

    storage::backend_factory::get().
        register_alias("NVML", backend_type::nvml);

    // register Lustre backend plugin
    storage::backend_factory::get().
        register_backend<storage::lustre>(
            backend_type::lustre,
            [](const std::string& nsid, bool track, 
               const bfs::path& mount, uint32_t quota) {
                return std::shared_ptr<storage::lustre>(
                        new storage::lustre(nsid, track, mount, quota));
            });
    storage::backend_factory::get().
        register_alias("Lustre", backend_type::lustre);
}

void urd::load_transfer_plugins() {

    LOGGER_INFO(" * Loading data transfer plugins...");

    try {
        m_transferor_registry = std::make_unique<io::transferor_registry>();
    }
    catch(const std::exception& e) {
        LOGGER_ERROR("Failed to create transfer plugin registry. This should "
                     "not happen under normal conditions.");
        exit(EXIT_FAILURE);
    }

    const auto load_plugin = [&](const data::resource_type t1, 
                                 const data::resource_type t2, 
                          std::shared_ptr<io::transferor>&& trp) {

#if 0
        // if(!m_transferor_registry->add(t1, t2, 
        //             std::forward<std::shared_ptr<io::transferor>>(trp))) {

        //     LOGGER_WARN("    Failed to load transfer plugin ({} to {}):\n"
        //                 "    Another plugin was already registered for this "
        //                 "    combination (Plugin ignored)",
        //                 utils::to_string(t1), utils::to_string(t2));
        //     return;
        // }

#else
        if(!m_task_mgr->register_transfer_plugin(t1, t2,
                    std::forward<std::shared_ptr<io::transferor>>(trp))) {
            LOGGER_WARN("    Failed to load transfer plugin ({} to {}):\n"
                        "    Another plugin was already registered for this\n"
                        "    combination (Plugin ignored)",
                        utils::to_string(t1), utils::to_string(t2));
            return;
        }
#endif

        LOGGER_INFO("    Loaded transfer plugin ({} => {})", 
                    utils::to_string(t1), utils::to_string(t2));
    };

    context ctx(m_settings->staging_directory(),
                m_network_service);

    // memory region -> local path
    load_plugin(
        data::resource_type::memory_region, 
        data::resource_type::local_posix_path, 
        std::make_shared<io::memory_region_to_local_path_transferor>(ctx));

    // memory region -> shared path
    load_plugin(
        data::resource_type::memory_region, 
        data::resource_type::shared_posix_path, 
        std::make_shared<io::memory_region_to_shared_path_transferor>(ctx));

    // memory region -> remote path
    load_plugin(
        data::resource_type::memory_region,
        data::resource_type::remote_resource,
        std::make_shared<io::memory_region_to_remote_resource_transferor>(ctx));

    // local path -> local path
    load_plugin(
        data::resource_type::local_posix_path, 
        data::resource_type::local_posix_path, 
        std::make_shared<io::local_path_to_local_path_transferor>(ctx));

    // local path -> shared path
    load_plugin(
        data::resource_type::local_posix_path, 
        data::resource_type::shared_posix_path, 
        std::make_shared<io::local_path_to_shared_path_transferor>(ctx));

    // local path -> remote resource
    load_plugin(
        data::resource_type::local_posix_path, 
        data::resource_type::remote_resource, 
        std::make_shared<io::local_path_to_remote_resource_transferor>(ctx));

    // remote resource -> local path
    load_plugin(
        data::resource_type::remote_resource, 
        data::resource_type::local_posix_path, 
        std::make_shared<io::remote_resource_to_local_path_transferor>(ctx));
}

void urd::load_default_namespaces() {

    LOGGER_INFO(" * Loading default namespaces...");

    for(const auto& nsdef: m_settings->default_namespaces()) {
        if(create_namespace(nsdef) != urd_error::success) {
            LOGGER_WARN("    Failed to load namespace \"{}://\" -> {} "
                        "of type {}: Ignored", nsdef.nsid(), 
                        nsdef.mountpoint(), nsdef.alias());
            continue;
        }
        LOGGER_INFO("    Loaded namespace \"{}://\" -> {} (type: {}, {})", 
                    nsdef.nsid(), nsdef.mountpoint(), nsdef.alias(), 
                    (nsdef.track() ? "tracked" : "untracked"));
    }
}

void
urd::check_configuration() {

    // check that the staging directory exists and that we can write to it
    if(!bfs::exists(m_settings->staging_directory())) {
        LOGGER_ERROR("Staging directory {} does not exist", 
                     m_settings->staging_directory());
        teardown_and_exit();
    }

    auto s = bfs::status(m_settings->staging_directory()); 

    auto expected_perms = bfs::perms::owner_read | 
                          bfs::perms::owner_write;
    
    if((s.permissions() & expected_perms) != expected_perms) {
        LOGGER_ERROR("Unable to read from/write to staging directory {}",
                     m_settings->staging_directory());
        teardown_and_exit();
    }
}

void urd::print_greeting() {
    const char greeting[] = "Starting {} daemon (pid {})";
    const auto gsep = std::string(sizeof(greeting) - 4 + 
                                  m_settings->progname().size() + 
                                  std::to_string(getpid()).size(), '=');

    LOGGER_INFO("{}", gsep);
    LOGGER_INFO(greeting, m_settings->progname(), getpid());
    LOGGER_INFO("{}", gsep);
}

void urd::print_configuration() {
    LOGGER_INFO("");
    LOGGER_INFO("[[ Configuration ]]");
    LOGGER_INFO("  - running as daemon?: {}", (m_settings->daemonize() ? "yes" : "no"));

    if(!m_settings->log_file().empty()) {
        LOGGER_INFO("  - log file: {}", m_settings->log_file());
        LOGGER_INFO("  - log file maximum size: {}", m_settings->log_file_max_size());
    }
    else {
        LOGGER_INFO("  - log file: none");
    }

    LOGGER_INFO("  - dry run?: {} [duration: {} microseconds]", 
            (m_settings->dry_run() ? "yes" : "no"), m_settings->dry_run_duration());
    LOGGER_INFO("  - pidfile: {}", m_settings->pidfile());
    LOGGER_INFO("  - control socket: {}", m_settings->control_socket());
    LOGGER_INFO("  - global socket: {}", m_settings->global_socket());
    LOGGER_INFO("  - staging directory: {}", m_settings->staging_directory());
    LOGGER_INFO("  - port for remote requests: {}", m_settings->remote_port());
    LOGGER_INFO("  - workers: {}", m_settings->workers_in_pool());
    LOGGER_INFO("");
}

void urd::print_farewell() {
    const char farewell[] = "Stopping {} daemon (pid {})";
    const auto fsep = std::string(sizeof(farewell) - 4 + 
                                  m_settings->progname().size() + 
                                  std::to_string(getpid()).size(), '=');

    LOGGER_INFO("{}", fsep);
    LOGGER_INFO(farewell, m_settings->progname(), getpid());
    LOGGER_INFO("{}", fsep);
}

void urd::pause_listening() {
    bool expected = false;
    while(!m_is_paused.compare_exchange_weak(expected, true) && !expected);

    LOGGER_WARN("Daemon locked: incoming requests will be rejected");
}

void urd::resume_listening() {
    bool expected = true;
    while(!m_is_paused.compare_exchange_weak(expected, false) && expected);

    LOGGER_WARN("Daemon unlocked: incoming requests will be processed");
}

urd_error urd::check_shutdown() {
    // - if there are active tasks (i.e. pending or running), we let 
    // the client know by returning urd_error::tasks_pending
    const auto task_is_active = 
        [](const std::shared_ptr<io::task_info>& ti) {
            // make sure no modifications can happen 
            // to the task metadata while we examine it
            const auto lock = ti->lock_shared();
            return (ti->status() == io::task_status::pending ||
                    ti->status() == io::task_status::running);
        };

    if(m_task_mgr->count_if(task_is_active) != 0) {
        return urd_error::tasks_pending;
    }

    // - if there are no active tasks but non-empty tracked backends 
    // remain, we return urd_error::namespace_not_empty
    const auto tracked_namespace_not_empty =
        [](const std::shared_ptr<storage::backend>& b) {
            // no need to filter out the process_memory and remote backends,
            // since they will never be tracked
            return (b->is_tracked() && !b->is_empty());
        };

    boost::shared_lock<boost::shared_mutex> lock(m_namespace_mgr_mutex);
    if(m_namespace_mgr->count_if(tracked_namespace_not_empty) != 0) {
        return urd_error::namespace_not_empty;
    }

    // - otherwise, we return urd_error::success
    return urd_error::success;
}

int urd::run() {

    // initialize logging facilities
    init_logger();

    // validate settings
    check_configuration();

#ifdef __LOGGER_ENABLE_DEBUG__
    if(::prctl(PR_SET_DUMPABLE, 1) != 0) {
        LOGGER_WARN("Failed to set PR_SET_DUMPABLE flag for process. "
                    "Daemon will not produce core dumps.");
    }
#endif

    // daemonize if needed
    if(m_settings->daemonize() && daemonize() != 0) {
        /* parent clean ups and exits, child continues */
        teardown();
        return EXIT_SUCCESS;
    }

    // print useful information
    print_greeting();
    print_configuration();

    LOGGER_INFO("[[ Starting up ]]");

    init_task_manager();
    init_event_handlers();
    init_namespace_manager();
    load_backend_plugins();
    load_default_namespaces();

    // load plugins now so that when we propagate the daemon context to them 
    // everything is set up
    load_transfer_plugins();

    // start the listener for remote transfers
    // N.B. This call returns immediately
    m_network_service->run();

    LOGGER_INFO("");
    LOGGER_INFO("[[ Start up successful, awaiting requests... ]]");

    // N.B. This call blocks here, which means that everything after it
    // will only run when a shutdown command is received
    m_ipc_service->run();

    print_farewell();
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

    if(m_ipc_service) {
        LOGGER_INFO("* Stopping API listener...");
        m_ipc_service->stop();
        m_ipc_service.reset();
    }

    api_listener::cleanup();

    if(m_settings) {
        boost::system::error_code ec;

        bfs::remove(m_settings->pidfile(), ec);

        if(ec) {
            LOGGER_ERROR("Failed to remove pidfile {}: {}", 
                    m_settings->pidfile(), ec.message());
        }

        m_settings.reset();
    }

    if(m_task_mgr) {
        LOGGER_INFO("* Stopping task manager...");
        m_task_mgr->stop_all_tasks();
        m_task_mgr.reset();
    }
}

void
urd::teardown_and_exit() {
    teardown();
    exit(EXIT_FAILURE);
}

void urd::shutdown() {
    if(m_ipc_service) {
        m_ipc_service->stop();
    }
}

} // namespace norns
