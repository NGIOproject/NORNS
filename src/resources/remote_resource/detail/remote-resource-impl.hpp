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

#ifndef __REMOTE_RESOURCE_IMPL_HPP__
#define __REMOTE_RESOURCE_IMPL_HPP__

#include <string>
#include <memory>
#include <vector>
#include <system_error>
#include <hermes.hpp>

namespace norns {

// forward declarations
namespace storage {
namespace detail {
class remote_backend;
}
}

namespace data {

enum class resource_type;

namespace detail {

template <>
struct resource_impl<resource_type::remote_resource> : public resource {

    resource_impl(const std::shared_ptr<const storage::backend> parent, 
                  const std::shared_ptr<const remote_resource_info> rinfo);

    std::string name() const override final;
    resource_type type() const override final;
    bool is_collection() const override final;
    std::size_t packed_size() const override final;
    const std::shared_ptr<const storage::backend> parent() const override final;
    std::string to_string() const override final;

    std::string
    address() const;

    std::string
    nsid() const;

    bool
    has_buffer() const;

    hermes::exposed_memory
    buffers() const;

    const std::string m_name;
    const std::string m_address;
    const std::string m_nsid;
    const hermes::exposed_memory m_buffers;
    const bool m_is_collection;
    const std::shared_ptr<const storage::detail::remote_backend> m_parent;
};

} // namespace detail
} // namespace data
} // namespace norns

#endif /* __REMOTE_RESOURCE_IMPL_HPP__ */
