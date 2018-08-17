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

#ifndef __DEFAULTS_HPP__
#define __DEFAULTS_HPP__

#include <cstdint>
#include <netinet/in.h>

namespace norns {
namespace config {
namespace defaults {

    extern const char*      progname;
    extern const bool       daemonize;
    extern const bool       use_syslog;
    extern const bool       dry_run;
    extern const char*      global_socket;
    extern const char*      control_socket;
    extern const in_port_t  remote_port;
    extern const char*      pidfile;
    extern const uint32_t   workers_in_pool;
    extern const uint32_t   backlog_size;
    extern const char*      config_file;

} // namespace defaults
} // namespace config
} // namespace norns

#endif /* __DEFAULTS_HPP__ */
