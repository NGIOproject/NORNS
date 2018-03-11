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

#include "settings.hpp"
#include "backends.hpp"
#include "signal-listener.hpp"
#include "logger.hpp"
#include "api.hpp"

#include "thread-pool.hpp"

#include "job.hpp"

#include "task-manager.hpp"



namespace norns {


/*! Aliases for convenience */
using signal_listener_ptr = std::unique_ptr<signal_listener>;
using thread_pool_ptr = std::unique_ptr<thread_pool>;

using api_listener = api::listener<api::message<api::request, api::response>>;
using api_listener_ptr = std::unique_ptr<api_listener>;

using request_ptr = std::unique_ptr<api::request>;
using response_ptr = std::unique_ptr<api::response>;

using backend_ptr = std::shared_ptr<storage::backend>;
using backend_manager = std::unordered_map<std::string, backend_ptr>;
using backend_manager_ptr = std::unique_ptr<backend_manager>;

using resource_info_ptr = std::shared_ptr<data::resource_info>;
using resource_ptr = std::shared_ptr<data::resource>;

// forward declarations
namespace io {
    struct task_stats;
}

enum class urd_error;



class urd {

public:
    void configure(const config_settings& settings);
    int run();
    void teardown();

private:
    int daemonize();
    void signal_handler(int);

    urd_error validate_iotask_args(iotask_type type, 
                                   resource_info_ptr src_info, 
                                   resource_info_ptr dst_info) const;

    response_ptr iotask_create_handler(const request_ptr req);
    response_ptr iotask_status_handler(const request_ptr req) const;
    response_ptr ping_handler(const request_ptr req);
    response_ptr job_register_handler(const request_ptr req);
    response_ptr job_update_handler(const request_ptr req);
    response_ptr job_remove_handler(const request_ptr req);
    response_ptr process_add_handler(const request_ptr req);
    response_ptr process_remove_handler(const request_ptr req);
    response_ptr backend_register_handler(const request_ptr req);
    response_ptr backend_update_handler(const request_ptr req);
    response_ptr backend_remove_handler(const request_ptr req);
    response_ptr unknown_request_handler(const request_ptr req);

private:
    signal_listener_ptr                    m_signal_listener;
    std::shared_ptr<config_settings>                    m_settings;
    thread_pool_ptr m_workers;



    api_listener_ptr                                    m_api_listener;
//    std::list<std::shared_ptr<storage::backend>>        m_backends;

    backend_manager_ptr m_backends;
    boost::shared_mutex m_backends_mutex;


    std::unordered_map<uint32_t, std::shared_ptr<job>>    m_jobs;
    boost::shared_mutex                         m_jobs_mutex;

    std::unique_ptr<task_manager> m_task_manager;
    mutable boost::shared_mutex   m_task_manager_mutex;
};

} // namespace norns 

#endif /* __URD_HPP__ */
