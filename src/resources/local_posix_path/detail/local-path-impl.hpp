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

#ifndef __LOCAL_PATH_IMPL_HPP__
#define __LOCAL_PATH_IMPL_HPP__

#include <string>
#include <memory>
#include <vector>
#include <system_error>
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

namespace norns {

// forward declarations
namespace storage {
class posix_filesystem;
}

namespace data {

enum class resource_type;

namespace detail {

template <>
struct resource_impl<resource_type::local_posix_path> : public resource {

    resource_impl(const std::shared_ptr<const storage::backend> parent, 
                  const bfs::path& name);

    std::string name() const override final;
    resource_type type() const override final;
    bool is_collection() const override final;
    std::size_t packed_size() const override final;
    const std::shared_ptr<const storage::backend> parent() const override final;
    std::string to_string() const override final;

    bfs::path canonical_path() const;

    const bfs::path m_name_in_namespace; // absolute pathname w.r.t. backend's mount point
    const bfs::path m_canonical_path; // canonical pathname
    const bool m_is_collection;
    const std::shared_ptr<const storage::posix_filesystem> m_parent;
};

} // namespace detail
} // namespace data
} // namespace norns

#endif /* __LOCAL_PATH_IMPL_HPP__ */
