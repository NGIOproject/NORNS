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
#include "remote-backend.hpp"

namespace norns {
namespace storage {
namespace detail {

remote_backend::remote_backend() {} 

std::string remote_backend::mount() const {
    return "";
}

uint32_t remote_backend::quota() const {
    return 0;
}

bool remote_backend::accepts(resource_info_ptr res) const {
    switch(res->type()) {
        case data::resource_type::local_posix_path:
            return true;
        default:
            return false;
    }
}

bool remote_backend::contains(resource_info_ptr res) const {
    return false;
}

void remote_backend::read_data() const {
}

void remote_backend::write_data() const {
}

std::string remote_backend::to_string() const {
    return "REMOTE_BACKEND(";// + m_mount + ", " + std::to_string(m_quota) + ")";
}

} // namespace detail
} // namespace storage
} // namespace norns
