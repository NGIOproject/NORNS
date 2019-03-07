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

#ifndef __URD_HPP__
#define __URD_HPP__

#include <unordered_map>
#include <boost/thread/shared_mutex.hpp>

#include "common.hpp"
#include "config.hpp"
#include "backends.hpp"
#include "logger.hpp"
#include "api.hpp"

#include "job.hpp"

namespace hermes {
    class async_engine;
    template <typename T> class request;
}

namespace norns {


/*! Aliases for convenience */
using api_listener = api::listener<api::message<api::request, api::response>>;

using request_ptr = std::unique_ptr<api::request>;
using response_ptr = std::unique_ptr<api::response>;

using backend_ptr = std::shared_ptr<storage::backend>;
using namespace_manager = std::unordered_map<std::string, backend_ptr>;

using resource_info_ptr = std::shared_ptr<data::resource_info>;
using resource_ptr = std::shared_ptr<data::resource>;


// forward declarations
namespace io {
    struct transferor_registry;
    struct task_manager;
    struct task_stats;
}

namespace ns {
    struct namespace_manager;
}

namespace rpc {
    struct push_resource;
    struct pull_resource;
    struct stat_resource;
}

enum class urd_error;

class urd {

public:
    urd();
    ~urd();
    void configure(const config::settings& settings);
    config::settings get_configuration() const;
    int run();
    void shutdown();
    void teardown();
    void teardown_and_exit();

private:
    int daemonize();
    void signal_handler(int);

    void init_logger();
    void init_event_handlers();
    void init_namespace_manager();
    void init_task_manager();
    void load_backend_plugins();
    void load_transfer_plugins();
    void load_default_namespaces();
    void check_configuration();
    void print_greeting();
    void print_configuration();
    void print_farewell();

    urd_error validate_iotask_args(iotask_type type, 
                                   const resource_info_ptr& src_info, 
                                   const resource_info_ptr& dst_info) const;

    response_ptr iotask_create_handler(const request_ptr req);
    response_ptr iotask_status_handler(const request_ptr req) const;
    response_ptr ping_handler(const request_ptr req);
    response_ptr job_register_handler(const request_ptr req);
    response_ptr job_update_handler(const request_ptr req);
    response_ptr job_remove_handler(const request_ptr req);
    response_ptr process_add_handler(const request_ptr req);
    response_ptr process_remove_handler(const request_ptr req);
    response_ptr namespace_register_handler(const request_ptr req);
    response_ptr namespace_update_handler(const request_ptr req);
    response_ptr namespace_remove_handler(const request_ptr req);
    response_ptr global_status_handler(const request_ptr req);
    response_ptr command_handler(const request_ptr req);
    response_ptr unknown_request_handler(const request_ptr req);

    void push_resource_handler(hermes::request<rpc::push_resource>&& req);
    void pull_resource_handler(hermes::request<rpc::pull_resource>&& req);
    void stat_resource_handler(hermes::request<rpc::stat_resource>&& req);

    // TODO: add helpers for remove and update
    urd_error create_namespace(const config::namespace_def& nsdef);
    urd_error create_namespace(const std::string& nsid, backend_type type,
                               bool track, const bfs::path& mount, 
                               uint32_t quota);

    void pause_listening();
    void resume_listening();
    urd_error check_shutdown();


private:
    std::atomic<bool> m_is_paused;

    std::shared_ptr<config::settings>                    m_settings;
    std::unique_ptr<io::transferor_registry> m_transferor_registry;

    std::unique_ptr<api_listener> m_ipc_service;

    std::shared_ptr<hermes::async_engine> m_network_service;

    std::unique_ptr<ns::namespace_manager> m_namespace_mgr;
    mutable boost::shared_mutex m_namespace_mgr_mutex;


    std::unordered_map<uint32_t, std::shared_ptr<job>>    m_jobs;
    boost::shared_mutex                         m_jobs_mutex;

    std::unique_ptr<io::task_manager> m_task_mgr;
    mutable boost::shared_mutex  m_task_mgr_mutex;
};

} // namespace norns 

#endif /* __URD_HPP__ */
