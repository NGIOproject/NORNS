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

#include <system_error>
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

#include "resource-type.hpp"
#include "resource.hpp"
#include "shared-path-info.hpp"
#include "shared-path-impl.hpp"
#include "backends/posix-fs.hpp"

namespace norns {
namespace data {
namespace detail {

// local alias for convenience
using shared_path_resource = resource_impl<resource_type::shared_posix_path>;

shared_path_resource::resource_impl(const std::shared_ptr<const storage::backend> parent, 
                                    const bfs::path& name) :
    m_name_in_namespace(name),
    m_canonical_path(parent->mount() / name),
    m_is_collection(bfs::is_directory(m_canonical_path)),
    m_parent(std::static_pointer_cast<const storage::posix_filesystem>(std::move(parent))) { }

std::string shared_path_resource::name() const {
    return m_name_in_namespace.string();
}

resource_type shared_path_resource::type() const {
    return resource_type::shared_posix_path;
}

bool shared_path_resource::is_collection() const {
    return m_is_collection;
}

std::size_t
shared_path_resource::packed_size() const {
    return 0;
}

const std::shared_ptr<const storage::backend>
shared_path_resource::parent() const {
    return std::static_pointer_cast<const storage::backend>(m_parent);
}

std::string shared_path_resource::to_string() const {
    return "/foo/bar/baz"; // TODO
}

} // namespace detail
} // namespace data
} // namespace norns
