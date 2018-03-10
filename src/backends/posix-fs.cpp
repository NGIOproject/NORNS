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
#include "posix-fs.hpp"

namespace storage {

posix_filesystem::posix_filesystem(const std::string& mount, uint32_t quota) 
    : m_mount(mount), m_quota(quota) { }

std::string posix_filesystem::mount() const {
    return m_mount;
}

uint32_t posix_filesystem::quota() const {
    return m_quota;
}

bool posix_filesystem::accepts(resource_info_ptr res) const {
    switch(res->type()) {
        case data::resource_type::local_posix_path:
            return true;
        default:
            return false;
    }
}

bool posix_filesystem::contains(resource_info_ptr res) const {
    return true; //XXX do actual check
}

void posix_filesystem::read_data() const {
}

void posix_filesystem::write_data() const {
}

std::string posix_filesystem::to_string() const {
    return "POSIX_FILESYSTEM(" + m_mount + ", " + std::to_string(m_quota) + ")";
}

} // namespace storage
