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
