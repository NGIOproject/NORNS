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

#include "common.hpp"
#include "resource-type.hpp"
#include "resource.hpp"
#include "memory-region-info.hpp"
#include "memory-region-impl.hpp"
#include "backends/process-memory.hpp"

namespace norns {
namespace data {
namespace detail {

// local alias for convenience
using memory_region_resource = resource_impl<resource_type::memory_region>;

memory_region_resource::resource_impl(const std::shared_ptr<const storage::backend> parent) :
    m_parent(std::static_pointer_cast<const storage::detail::process_memory>(std::move(parent))) { }

std::string memory_region_resource::name() const {
    return "";
}

resource_type memory_region_resource::type() const {
    return resource_type::memory_region;
}

bool memory_region_resource::is_collection() const {
    return false;
}

const std::shared_ptr<const storage::backend>
memory_region_resource::parent() const {
    return std::static_pointer_cast<const storage::backend>(m_parent);
}

std::string memory_region_resource::to_string() const {
    return "0x42+42"; // TODO
}

} // namespace detail
} // namespace data
} // namespace norns
