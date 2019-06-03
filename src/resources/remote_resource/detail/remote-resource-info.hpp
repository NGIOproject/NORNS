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

#ifndef __REMOTE_RESOURCE_INFO_HPP__
#define __REMOTE_RESOURCE_INFO_HPP__

#include <hermes.hpp>
#include <string>
#include "resource-info.hpp"
#include "rpcs.hpp"

namespace norns {
namespace data {

enum class resource_type;

namespace detail {

/*! Local filesystem path data */
struct remote_resource_info : public resource_info {

    remote_resource_info(const std::string& address,
                         const std::string& nsid, 
                         const std::string& name);

    remote_resource_info(const std::string& address,
                         const std::string& nsid, 
                         bool is_collection,
                         const std::string& name,
                         const hermes::exposed_memory& buffers);
    ~remote_resource_info();
    resource_type type() const override final;
    std::string nsid() const override final;
    bool is_remote() const override final;
    std::string to_string() const override final;

    std::string
    address() const;

    std::string
    name() const;

    bool
    is_collection() const;

    bool
    has_buffers() const;

    hermes::exposed_memory
    buffers() const;

    std::string m_address;
    std::string m_nsid;
    bool m_is_collection;
    std::string m_name;
    hermes::exposed_memory m_buffers;
};

} // namespace detail
} // namespace data
} // namespace norns

#endif // __REMOTE_RESOURCE_INFO_HPP__
