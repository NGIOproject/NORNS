/************************************************************************* 
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of NORNS, a service that allows other programs to   *
 * start, track and manage asynchronous transfers of data resources      *
 * between different storage backends.                                   *
 *                                                                       *
 * See AUTHORS file in the top level directory for information regarding *
 * developers and contributors.                                          *
 *                                                                       *
 * This software was developed as part of the EC H2020 funded project    *
 * NEXTGenIO (Project ID: 671951).                                       *
 *     www.nextgenio.eu                                                  *
 *                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining *
 * a copy of this software and associated documentation files (the       *
 * "Software"), to deal in the Software without restriction, including   *
 * without limitation the rights to use, copy, modify, merge, publish,   *
 * distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to *
 * the following conditions:                                             *
 *                                                                       *
 * The above copyright notice and this permission notice shall be        *
 * included in all copies or substantial portions of the Software.       *
 *                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                 *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS   *
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN    *
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN     *
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE      *
 * SOFTWARE.                                                             *
 *************************************************************************/

#ifndef __DEFAULTS_HPP__
#define __DEFAULTS_HPP__

#include <cstdint>
#include <netinet/in.h>
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

namespace norns {
namespace config {
namespace defaults {

    extern const char*      progname;
    extern const bool       daemonize;
    extern const bool       use_syslog;
    extern const bool       use_console;
    extern const bfs::path  log_file;
    extern const uint32_t   log_file_max_size;
    extern const bool       dry_run;
    extern const uint32_t   dry_run_duration;
    extern const char*      global_socket;
    extern const char*      control_socket;
    extern const char*      bind_address;
    extern const in_port_t  remote_port;
    extern const char*      pidfile;
    extern const uint32_t   workers_in_pool;
    extern const char*      staging_directory;
    extern const uint32_t   backlog_size;
    extern const char*      config_file;

} // namespace defaults
} // namespace config
} // namespace norns

#endif /* __DEFAULTS_HPP__ */
