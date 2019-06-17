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

#ifndef __RESOURCE_TYPE_HPP__
#define __RESOURCE_TYPE_HPP__

#include <string>

namespace norns {
namespace data {

/*! Supported resource types */
enum class resource_type {
    memory_region,
    local_posix_path,
    shared_posix_path,
    remote_posix_path,
    remote_resource,
    ignorable
};

} // namespace data

namespace utils {

static inline std::string to_string(data::resource_type type) {
    switch(type) {
        case data::resource_type::memory_region:
            return "MEMORY_REGION";
        case data::resource_type::local_posix_path:
            return "LOCAL_PATH";
        case data::resource_type::shared_posix_path:
            return "SHARED_PATH";
        case data::resource_type::remote_posix_path:
            return "REMOTE_PATH";
        case data::resource_type::remote_resource:
            return "REMOTE_RESOURCE";
        default:
            return "UNKNOWN_RESOURCE_TYPE";
    }
}

}

} // namespace norns

#endif /* __RESOURCE_TYPE_HPP__ */
