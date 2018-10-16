/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of the NORNS Data Scheduler, a service that allows  *
 * other programs to start, track and manage asynchronous transfers of   *
 * data resources transfers requests between different storage backends. *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The NORNS Data Scheduler is free software: you can redistribute it    *
 * and/or modify it under the terms of the GNU Lesser General Public     *
 * License as published by the Free Software Foundation, either          *
 * version 3 of the License, or (at your option) any later version.      *
 *                                                                       *
 * The NORNS Data Scheduler is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with the NORNS Data Scheduler.  If not, see      *
 * <http://www.gnu.org/licenses/>.                                       *
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
