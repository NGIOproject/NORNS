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

#ifndef __IO_TRANSFEROR_REGISTRY_HPP__
#define __IO_TRANSFEROR_REGISTRY_HPP__

#include <system_error>
#include <utility>
#include <boost/functional/hash.hpp>
#include "auth/process-credentials.hpp"
#include "common.hpp"

namespace norns {

// forward declarations
namespace data { 
enum class resource_type;
}

namespace io {

struct transferor;

struct transferor_registry {

    bool add(const data::resource_type t1, 
             const data::resource_type t2, 
             std::shared_ptr<io::transferor>&& tr);
    std::shared_ptr<io::transferor> get(const data::resource_type t1, 
                                        const data::resource_type t2) const;

    using key_type = 
        std::pair<const data::resource_type, const data::resource_type>;

    std::unordered_map<key_type,
                       std::shared_ptr<io::transferor>, 
                       boost::hash<key_type>> m_transferors;
};

} // namespace io
} // namespace norns

#endif /* __IO_TRANSFEROR_REGISTRY_HPP__ */

