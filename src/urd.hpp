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

#include <sys/socket.h>
#include <sys/un.h>
#include <ev.h>

#include "settings.hpp"
#include "backends.hpp"
#include "signal-listener.hpp"
#include "message.hpp"
#include "ipc-listener.hpp"
#include "logger.hpp"
#include "ctpl.h"
#include "requests.hpp"

class urd {

public:
    void set_configuration(const config_settings& settings);
    void run();

private:
    void daemonize();
    //void new_request_handler(struct norns_iotd*);
    void new_request_handler(message* msg);
    void request_handler(std::shared_ptr<urd_request> request);
    void signal_handler(int);

private:
    std::shared_ptr<logger>                          m_logger;
    std::shared_ptr<signal_listener>                 m_signal_listener;
    std::shared_ptr<config_settings>                 m_settings;
    std::shared_ptr<ctpl::thread_pool>               m_workers;
    //std::shared_ptr<ipc_listener<struct norns_iotd>> m_ipc_listener;
    std::shared_ptr<ipc_listener<message, urd_request>> m_ipc_listener;
    std::list<std::shared_ptr<storage::backend>>     m_backends;

};

#endif /* __URD_HPP__ */
