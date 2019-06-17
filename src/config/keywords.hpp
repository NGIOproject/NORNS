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

#ifndef __CONFIG_KEYWORDS_HPP__
#define __CONFIG_KEYWORDS_HPP__

namespace norns {
namespace config {

// define some constant keywords so that we can refer to them throughout
// the parsing code without having to rely on the actual string literal
// (this will allow us to rename options in the configuration file
// without having to modify all the statements that refer to them)
namespace keywords {

// section names
constexpr static const auto global_settings = "global_settings";
constexpr static const auto namespaces = "namespaces";

// option names for 'global-settings' section
constexpr static const auto use_syslog = "use_syslog";
constexpr static const auto log_file = "log_file";
constexpr static const auto log_file_max_size = "log_file_max_size";
constexpr static const auto dry_run = "dry_run";
constexpr static const auto global_socket = "global_socket";
constexpr static const auto control_socket = "control_socket";
constexpr static const auto bind_address = "bind_address";
constexpr static const auto remote_port = "remote_port";
constexpr static const auto pidfile = "pidfile";
constexpr static const auto workers = "workers";
constexpr static const auto staging_directory = "staging_directory";

// option names for 'namespaces' section
constexpr static const auto nsid = "nsid";
constexpr static const auto track_contents = "track_contents";
constexpr static const auto mountpoint = "mountpoint";
constexpr static const auto type = "type";
constexpr static const auto capacity = "capacity";
constexpr static const auto visibility = "visibility";

}

} // namespace config
} // namespace norns

#endif /* __CONFIG_KEYWORDS_HPP__ */

