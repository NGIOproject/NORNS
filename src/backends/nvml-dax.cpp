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

#include "backend-base.hpp"
#include "resources.hpp"
#include "utils.hpp"
#include "nvml-dax.hpp"

namespace norns {
namespace storage {

nvml_dax::nvml_dax(bool track, const bfs::path& mount, uint32_t quota) 
    : m_track(track),
      m_mount(mount),
      m_quota(quota) { }

bool
nvml_dax::is_tracked() const {
    return m_track;
}

bfs::path nvml_dax::mount() const {
    return m_mount;
}

uint32_t nvml_dax::quota() const {
    return m_quota;
}

backend::resource_ptr 
nvml_dax::new_resource(const resource_info_ptr& rinfo, 
                       const bool is_collection, 
                       std::error_code& ec) const {
    (void) rinfo;
    (void) is_collection;
    (void) ec;
    return std::make_shared<data::local_path_resource>(shared_from_this(), ""); //XXX
}

backend::resource_ptr
nvml_dax::get_resource(const resource_info_ptr& rinfo, std::error_code& ec) const {
    (void) rinfo;
    (void) ec;
    return std::make_shared<data::local_path_resource>(shared_from_this(), ""); //XXX
}

void
nvml_dax::remove(const resource_info_ptr& rinfo, std::error_code& ec) const {
    (void) rinfo;
    (void) ec;
}

std::size_t
nvml_dax::get_size(const resource_info_ptr& rinfo, std::error_code& ec) const {
    (void) rinfo;
    (void) ec;

    return 0; //XXX
}

bool nvml_dax::accepts(resource_info_ptr res) const {
    switch(res->type()) {
        case data::resource_type::local_posix_path:
            return true;
        default:
            return false;
    }
}

std::string nvml_dax::to_string() const {
    return "NORNS_NVML(\"" + m_mount.string() + "\", " + std::to_string(m_quota) + ")";
}

} // namespace storage
} // namespace norns
