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
#include "backends.hpp"
#include "memory-buffer.hpp"

namespace norns {
namespace data {

/*! Memory buffer data */
memory_buffer::memory_buffer(uint64_t address, std::size_t size)
    : m_address(address),
      m_size(size) {}

memory_buffer::~memory_buffer() { }

resource_type memory_buffer::type() const {
    return resource_type::memory_region;
}

std::string memory_buffer::nsid() const {
    return "";
}

bool memory_buffer::is_remote() const {
    return false;
}

std::string memory_buffer::to_string() const {
    std::stringstream ss;
    ss << "0x" << std::hex << m_address << "+" << "0x" << m_size;
    return "MEMBUF[" + ss.str() + "]";
}

namespace detail {

memory_region_resource::resource_impl(std::shared_ptr<resource_info> base_info) :
    m_backend(),
    m_resource_info(std::static_pointer_cast<memory_buffer>(base_info)) { }

std::string memory_region_resource::to_string() const {
    return m_backend->to_string() + m_resource_info->to_string();
}

//resource_type memory_region_resource::type() const {
//    return resource_type::memory_region;
//}

std::shared_ptr<resource_info> memory_region_resource::info() const {
    return m_resource_info;
}

std::shared_ptr<storage::backend> memory_region_resource::backend() const {
    return m_backend;
}

void memory_region_resource::set_backend(const std::shared_ptr<storage::backend> backend) {
    m_backend = backend;
}

/* Stream implementation */
memory_region_stream::stream_impl(std::shared_ptr<resource> resource) {
    (void) resource;
}

std::size_t memory_region_stream::read(buffer& b) {
    (void) b;
    return 0;
}

std::size_t memory_region_stream::write(const buffer& b) {
    (void) b;
    return 0;
}

} // namespace detail

} // namespace data
} // namespace norns
