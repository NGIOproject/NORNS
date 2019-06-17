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

#include <boost/filesystem.hpp>
#include "logger.hpp"
#include "backend-base.hpp"

namespace bfs = boost::filesystem;

namespace norns {
namespace storage {

backend_factory& 
backend_factory::get() {
    static backend_factory _;
    return _;
}

std::shared_ptr<backend>
backend_factory::create(const backend_type type, 
                        const std::string& nsid, 
                        bool track, 
                        const bfs::path& mount, 
                        uint32_t quota) const {

    boost::system::error_code ec;

    bfs::path canonical_mount = bfs::canonical(mount, ec);

    if(ec) {
        LOGGER_ERROR("Invalid mount point: {}", ec.message());
        throw std::invalid_argument("");
    }

    const int32_t id = static_cast<int32_t>(type);

    const auto& it = m_registrar.find(id);

    if(it != m_registrar.end()){
        return std::shared_ptr<backend>(
                it->second(nsid, track, canonical_mount, quota));
    }
    else{
        throw std::invalid_argument("Unrecognized backend type!");
    }
}

} // namespace storage
} // namespace norns
