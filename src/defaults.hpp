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

#ifndef __DEFAULTS_HPP__
#define __DEFAULTS_HPP__

#include <thread>
#include <list>
#include <boost/property_tree/ptree.hpp>

#include "defaults.hpp"

namespace defaults {
    extern const char* progname;
    extern const bool  daemonize;
    extern const bool  detach;
    extern const char* running_dir;
    extern const char* ipc_sockfile;
    extern const char* daemon_pidfile;
    extern const uint32_t workers_in_pool;
    extern const char* config_file;
} // namespace defaults

#endif /* __DEFAULTS_HPP__ */
