/*************************************************************************
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
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

#include <system_error>

#include "common.hpp"
#include "resource-type.hpp"
#include "resource.hpp"
#include "remote-resource-info.hpp"
#include "remote-resource-impl.hpp"
#include "backends/remote-backend.hpp"
#include "logger.hpp"

namespace norns {
namespace data {
namespace detail {

// local alias for convenience
using remote_resource = resource_impl<resource_type::remote_resource>;

remote_resource::resource_impl(
        const std::shared_ptr<const storage::backend> parent, 
        const std::shared_ptr<const remote_resource_info> rinfo) :
    m_name(rinfo->m_name),
    m_address(rinfo->m_address),
    m_nsid(rinfo->m_nsid),
    m_buffers(rinfo->m_buffers),
    m_is_collection(rinfo->m_is_collection),
    m_parent(std::static_pointer_cast<const storage::detail::remote_backend>(std::move(parent))) { }

std::string
remote_resource::name() const {
    return m_name;
}

resource_type
remote_resource::type() const {
    return resource_type::remote_resource;
}

bool
remote_resource::is_collection() const {
    return m_is_collection;
}

const std::shared_ptr<const storage::backend>
remote_resource::parent() const {
    return std::static_pointer_cast<const storage::backend>(m_parent);
}

std::string
remote_resource::to_string() const {
    return "REMOTE_RESOURCE[" + m_nsid + "@" + m_address + ":" + m_name + "]";
}

std::string
remote_resource::address() const {
    return m_address;
}

std::string
remote_resource::nsid() const {
    return m_nsid;
}

bool
remote_resource::has_buffer() const {
    return m_buffers.count() != 0;
}

hermes::exposed_memory
remote_resource::buffers() const {
    return m_buffers;
}

} // namespace detail
} // namespace data
} // namespace norns
