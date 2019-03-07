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

#include "resource-type.hpp"
#include "resource-info.hpp"
#include "remote-path-info.hpp"

namespace norns {
namespace data {
namespace detail {

/*! Remote path data */
remote_path_info::remote_path_info(const std::string& nsid, 
                                   const std::string& hostname, 
                                   const std::string& datapath)
    : m_nsid(nsid),
      m_hostname(hostname),
      m_datapath(datapath) {}

remote_path_info::~remote_path_info() { }

resource_type remote_path_info::type() const {
    return resource_type::remote_posix_path;
}

std::string remote_path_info::nsid() const {
    return m_nsid;
}

bool remote_path_info::is_remote() const {
    return true;
}

std::string remote_path_info::to_string() const {
    return "REMOTE_PATH[\"" + m_hostname + "\", \"" + m_nsid + "\", \"" + m_datapath + "\"]";
}

std::string 
remote_path_info::hostname() const {
    return m_hostname;
}

std::string remote_path_info::datapath() const {
    return m_datapath;
}

} // namespace detail
} // namespace data
} // namespace norns
