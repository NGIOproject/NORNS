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

#ifndef __POSIX_FS_HPP__
#define __POSIX_FS_HPP__

#include <system_error>
#include <boost/filesystem.hpp>

#include "norns/norns_backends.h"
#include "backend-base.hpp"

namespace bfs = boost::filesystem;

namespace norns {
namespace storage {

class posix_filesystem final : public storage::backend {
public:
    posix_filesystem(const std::string& nsid, bool track, const bfs::path& mount, uint32_t quota);

    std::string nsid() const override final;
    bool is_tracked() const override final;
    bool is_empty() const override final;
    bfs::path mount() const override final;
    uint32_t quota() const override final;

    resource_ptr new_resource(const resource_info_ptr& rinfo, bool is_collection, std::error_code& ec) const override final;
    resource_ptr get_resource(const resource_info_ptr& rinfo, std::error_code& ec) const override final;
    void remove(const resource_info_ptr& rinfo, std::error_code& ec) const override final;
    std::size_t get_size(const resource_info_ptr& rinfo, std::error_code& ec) const override final;

    bool accepts(resource_info_ptr res) const override final;
    std::string to_string() const override final;

private:
    std::string m_nsid;
    bool        m_track;
    bfs::path   m_mount;
    uint32_t    m_quota;
};

//NORNS_REGISTER_BACKEND(backend_type::posix_filesystem, posix_filesystem);

} // namespace storage
} // namespace norns

#endif // __POSIX_FS_HPP__
