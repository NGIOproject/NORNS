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

#ifndef __URD_HPP__
#define __URD_HPP__

#include <map>
#include <boost/thread/shared_mutex.hpp>

#include "settings.hpp"
#include "backend-base.hpp"
#include "signal-listener.hpp"
#include "logger.hpp"
#include "api.hpp"
#include "ctpl.h"





#include "job.hpp"

/*! Aliases for convenience */
using api_listener = api::listener<api::message<api::request, api::response>>;
using api_listener_ptr = std::unique_ptr<api_listener>;
using request_ptr = std::unique_ptr<api::request>;
using response_ptr = std::unique_ptr<api::response>;


class urd {

public:
    void configure(const config_settings& settings);
    void run();
    void stop();

private:
    void daemonize();
    void signal_handler(int);

    response_ptr register_job(const request_ptr req);
    response_ptr update_job(const request_ptr req);
    response_ptr remove_job(const request_ptr req);
    response_ptr add_process(const request_ptr req);
    response_ptr remove_process(const request_ptr req);
    response_ptr create_task(const request_ptr req);

private:
    pid_t                                               m_pid;
    std::shared_ptr<logger>                             m_logger;
    std::shared_ptr<signal_listener>                    m_signal_listener;
    std::shared_ptr<config_settings>                    m_settings;
    std::shared_ptr<ctpl::thread_pool>                  m_workers;
    api_listener_ptr                                    m_api_listener;
    std::list<std::shared_ptr<storage::backend>>        m_backends;


    std::map<uint32_t, std::shared_ptr<job>>    m_jobs;
    boost::shared_mutex                         m_jobs_mutex;



};

#endif /* __URD_HPP__ */
