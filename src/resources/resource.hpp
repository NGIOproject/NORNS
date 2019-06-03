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

#ifndef __RESOURCE_HPP__
#define __RESOURCE_HPP__

#include <memory>
#include <vector>

namespace norns {

// forward declare storage::backend
namespace storage {
    class backend;
}

namespace data {

enum class resource_type;

/*! A fully qualified resource for which we have a description and the backend
 * that stores it */
struct resource : public std::enable_shared_from_this<resource> {
    virtual ~resource() {}
    virtual resource_type type() const = 0;
    virtual std::string name() const = 0;
    virtual bool is_collection() const = 0;
    virtual std::size_t packed_size() const = 0;
    virtual const std::shared_ptr<const storage::backend> parent() const = 0; 
    virtual std::string to_string() const = 0;
};

namespace detail {

/*! Generic template for resource implementations 
 * (a concrete specialization must be provided for each resource_type) */
template <resource_type RT>
struct resource_impl;

} // namespace detail
} // namespace data 
} // namespace norns

#endif /* __RESOURCE_HPP__ */
