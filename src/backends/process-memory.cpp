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

#include "backend-base.hpp"
#include "resources.hpp"
#include "process-memory.hpp"

namespace norns {
namespace storage {
namespace detail {

process_memory::process_memory(const std::string& nsid) :
    m_nsid(nsid) { }

std::string
process_memory::nsid() const {
    return m_nsid;
}

bool 
process_memory::is_tracked() const {
    return false;
}

bool
process_memory::is_empty() const {
    return false;
}

bfs::path process_memory::mount() const {
    return "";
}

uint32_t process_memory::quota() const {
    return 0;
}

backend::resource_ptr 
process_memory::new_resource(const resource_info_ptr& rinfo, 
                             const bool is_collection, 
                             std::error_code& ec) const {
    (void) rinfo;
    (void) is_collection;
    (void) ec;
    return nullptr;
}

backend::resource_ptr 
process_memory::get_resource(const resource_info_ptr& rinfo, 
                             std::error_code& ec) const {

    const auto d_rinfo = std::static_pointer_cast<data::memory_region_info>(rinfo);

    ec = std::error_code();
    return std::make_shared<data::memory_region_resource>(
            shared_from_this(), d_rinfo->address(), d_rinfo->size());
}

void
process_memory::remove(const resource_info_ptr& rinfo, std::error_code& ec) const {
    (void) rinfo;
    (void) ec;
}

std::size_t
process_memory::get_size(const resource_info_ptr& rinfo, std::error_code& ec) const {

    const auto d_rinfo = std::static_pointer_cast<data::memory_region_info>(rinfo);

    ec = std::error_code();
    return d_rinfo->size();
}

bool process_memory::accepts(resource_info_ptr res) const {
    switch(res->type()) {
        case data::resource_type::memory_region:
            return true;
        default:
            return false;
    }
}

std::string process_memory::to_string() const {
    return "PROCESS_MEMORY";
}

} // namespace detail
} // namespace storage
} // namespace norns

