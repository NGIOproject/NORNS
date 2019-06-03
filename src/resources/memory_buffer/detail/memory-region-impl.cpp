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

#include "common.hpp"
#include "utils.hpp"
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

memory_region_resource::resource_impl(const std::shared_ptr<const storage::backend> parent,
                                      const uint64_t address, const std::size_t size) :
    m_address(address),
    m_size(size),
    m_parent(std::static_pointer_cast<const storage::detail::process_memory>(std::move(parent))) { }

std::string memory_region_resource::name() const {
    return ""; // TODO?
}

resource_type memory_region_resource::type() const {
    return resource_type::memory_region;
}

bool memory_region_resource::is_collection() const {
    return false;
}

std::size_t
memory_region_resource::packed_size() const {
    return m_size;
}

const std::shared_ptr<const storage::backend>
memory_region_resource::parent() const {
    return std::static_pointer_cast<const storage::backend>(m_parent);
}

uint64_t memory_region_resource::address() const {
    return m_address;
}

std::size_t memory_region_resource::size() const {
    return m_size;
}

std::string memory_region_resource::to_string() const {
    return utils::n2hexstr(m_address) + std::to_string(m_size);
}

} // namespace detail
} // namespace data
} // namespace norns
