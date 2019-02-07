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

