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
#include "shared-path.hpp"

namespace data {

/*! Remote path data */
shared_path::shared_path(std::string nsid, std::string datapath)
    : m_nsid(nsid),
      m_datapath(datapath) {}

shared_path::~shared_path() { }

resource_type shared_path::type() const {
    return resource_type::shared_posix_path;
}

std::string shared_path::nsid() const {
    return m_nsid;
}

bool shared_path::is_remote() const {
    return false;
}

std::string shared_path::to_string() const {
    return "SHARED_PATH[\"" + m_nsid + "\", \"" + m_datapath + "\"]";
}

namespace detail {

shared_path_resource::resource_impl(std::shared_ptr<resource_info> base_info) :
    m_backend(),
    m_resource_info(std::static_pointer_cast<shared_path>(base_info)) { }

std::string shared_path_resource::to_string() const {
    return m_backend->to_string() + m_resource_info->to_string();
}

//resource_type shared_path_resource::type() const {
//    return resource_type::shared_posix_path;
//}

std::shared_ptr<resource_info> shared_path_resource::info() const {
    return m_resource_info;
}

std::shared_ptr<storage::backend> shared_path_resource::backend() const {
    return m_backend;
}

void shared_path_resource::set_backend(const std::shared_ptr<storage::backend> backend) {
    m_backend = backend;
}

/* Stream implementation */
shared_path_stream::stream_impl(std::shared_ptr<resource> resource) {
    (void) resource;
}

std::size_t shared_path_stream::read(buffer& b) {
    (void) b;
    return 0;
}

std::size_t shared_path_stream::write(const buffer& b) {
    (void) b;
    return 0;
}

} // namespace detail

} // namespace data
