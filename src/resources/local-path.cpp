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
#include "local-path.hpp"

namespace data {

/*! Remote path data */
local_path::local_path(std::string nsid, std::string datapath)
    : m_nsid(nsid),
      m_datapath(datapath) {}

local_path::~local_path() { }

resource_type local_path::type() const {
    return resource_type::local_posix_path;
}

std::string local_path::nsid() const {
    return m_nsid;
}

bool local_path::is_remote() const {
    return false;
}

std::string local_path::to_string() const {
    return "LOCAL_PATH[\"" + m_nsid + "\", \"" + m_datapath + "\"]";
}

namespace detail {

resource_impl<resource_type::local_posix_path>::resource_impl(std::shared_ptr<resource_info> base_info) :
    m_backend(),
    m_resource_info(std::static_pointer_cast<local_path>(base_info)) { }

std::string resource_impl<resource_type::local_posix_path>::to_string() const {
    return m_backend->to_string() + m_resource_info->to_string();
}

resource_type resource_impl<resource_type::local_posix_path>::type() const {
    return resource_type::local_posix_path;
}

void resource_impl<resource_type::local_posix_path>::set_backend(const backend_ptr backend) {
    m_backend = backend;
}

/* Stream implementation */
stream_impl<resource_type::local_posix_path>::stream_impl(std::shared_ptr<resource> resource) {
    (void) resource;
}

std::size_t stream_impl<resource_type::local_posix_path>::read(buffer& b) {
    (void) b;
    return 0;
}

std::size_t stream_impl<resource_type::local_posix_path>::write(const buffer& b) {
    (void) b;
    return 0;
}

} // namespace detail

} // namespace data
