/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 *                                                                       *
 * This file is part of the Data Scheduler, a daemon for tracking and    *
 * managing requests for asynchronous data transfer in a hierarchical    *
 * storage environment.                                                  *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The Data Scheduler is free software: you can redistribute it and/or   *
 * modify it under the terms of the GNU Lesser General Public License    *
 * as published by the Free Software Foundation, either version 3 of     *
 * the License, or (at your option) any later version.                   *
 *                                                                       *
 * The Data Scheduler is distributed in the hope that it will be useful, *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with Data Scheduler.  If not, see                *
 * <http://www.gnu.org/licenses/>.                                       *
 *                                                                       *
 *************************************************************************/

#include "backends.hpp"
#include "remote-path.hpp"

namespace data {

/*! Remote path data */
remote_path::remote_path(std::string nsid, std::string hostname, std::string datapath)
    : m_nsid(nsid),
      m_hostname(hostname),
      m_datapath(datapath) {}

remote_path::~remote_path() { }

resource_type remote_path::type() const {
    return resource_type::remote_posix_path;
}

std::string remote_path::nsid() const {
    return m_nsid;
}

bool remote_path::is_remote() const {
    return true;
}

std::string remote_path::to_string() const {
    return "REMOTE_PATH[\"" + m_hostname + "\", \"" + m_nsid + "\", \"" + m_datapath + "\"]";
}

namespace detail {

remote_path_resource::resource_impl(std::shared_ptr<resource_info> base_info) :
    m_backend(),
    m_resource_info(std::static_pointer_cast<remote_path>(base_info)) { }

std::string remote_path_resource::to_string() const {
    return m_backend->to_string() + m_resource_info->to_string();
}

//resource_type remote_path_resource::type() const {
//    return resource_type::remote_posix_path;
//}

std::shared_ptr<resource_info> remote_path_resource::info() const {
    return m_resource_info;
}

std::shared_ptr<storage::backend> remote_path_resource::backend() const {
    return m_backend;
}

void remote_path_resource::set_backend(const std::shared_ptr<storage::backend> backend) {
    m_backend = backend;
}

/* Stream implementation */
remote_path_stream::stream_impl(std::shared_ptr<resource> resource) {
    (void) resource;
}

std::size_t remote_path_stream::read(buffer& b) {
    (void) b;
    return 0;
}

std::size_t remote_path_stream::write(const buffer& b) {
    (void) b;
    return 0;
}

} // namespace detail

} // namespace data
