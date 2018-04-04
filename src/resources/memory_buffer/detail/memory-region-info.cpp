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

#include <sstream>
#include "resource-type.hpp"
#include "resource-info.hpp"
#include "memory-region-info.hpp"

namespace norns {
namespace data {
namespace detail {

/*! Remote path data */
memory_region_info::memory_region_info(uint64_t address, std::size_t size)
    : m_address(address),
      m_size(size) {}

memory_region_info::~memory_region_info() { }

resource_type memory_region_info::type() const {
    return resource_type::memory_region;
}

std::string memory_region_info::nsid() const {
    // TODO: define and use constants
    return "[[internal::memory]]";
}

bool memory_region_info::is_remote() const {
    return false;
}

std::string memory_region_info::to_string() const {
    std::stringstream ss;
    ss << "0x" << std::hex << m_address << "+" << "0x" << m_size;
    return "MEMBUF[" + ss.str() + "]";
}

uint64_t memory_region_info::address() const {
    return m_address;
}

std::size_t memory_region_info::size() const {
    return m_size;
}

} // namespace detail
} // namespace data
} // namespace norns
