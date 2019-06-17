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

#include "resource-type.hpp"
#include "resource-info.hpp"
#include "remote-resource-info.hpp"

namespace norns {
namespace data {
namespace detail {

/*! Remote path data */
remote_resource_info::remote_resource_info(
        const std::string& address,
        const std::string& nsid,
        const std::string& name) :
    m_address(address),
    m_nsid(nsid),
    m_is_collection(false),
    m_name(name) { }

remote_resource_info::remote_resource_info(
        const std::string& address,
        const std::string& nsid,
        bool is_collection,
        const std::string& name,
        const hermes::exposed_memory& buffers) :
    m_address(address),
    m_nsid(nsid),
    m_is_collection(is_collection),
    m_name(name),
    m_buffers(buffers) { }

remote_resource_info::~remote_resource_info() { }

resource_type 
remote_resource_info::type() const {
    return resource_type::remote_resource;
}

std::string
remote_resource_info::address() const {
    return m_address;
}

std::string 
remote_resource_info::nsid() const {
    return m_nsid;
}

bool 
remote_resource_info::is_remote() const {
    return true;
}

std::string 
remote_resource_info::to_string() const {
    return "REMOTE_RESOURCE[" + m_nsid + "@" + m_address + ":" + m_name + "]";
}

std::string
remote_resource_info::name() const {
    return m_name;
}

bool
remote_resource_info::is_collection() const {
    return m_is_collection;
}

bool
remote_resource_info::has_buffers() const {
    return m_buffers.count() != 0;
}

hermes::exposed_memory
remote_resource_info::buffers() const {
    return m_buffers;
}

} // namespace detail
} // namespace data
} // namespace norns
