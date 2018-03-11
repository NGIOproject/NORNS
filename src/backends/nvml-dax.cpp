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

#include <sstream>
#include <boost/foreach.hpp>
#include "backend-base.hpp"
#include "utils.hpp"
#include "nvml-dax.hpp"

namespace norns {
namespace storage {

nvml_dax::nvml_dax(const std::string& mount, uint32_t quota) 
    : m_mount(mount), m_quota(quota) { }

std::string nvml_dax::mount() const {
    return m_mount;
}

uint32_t nvml_dax::quota() const {
    return m_quota;
}

bool nvml_dax::accepts(resource_info_ptr res) const {
    switch(res->type()) {
        case data::resource_type::local_posix_path:
            return true;
        default:
            return false;
    }
}

bool nvml_dax::contains(resource_info_ptr res) const {
    return true; //XXX do actual check
}

void nvml_dax::read_data() const {
}

void nvml_dax::write_data() const {
}

std::string nvml_dax::to_string() const {
    std::stringstream ss;

    ss << "{NORNS_NVML, " << m_mount << ", " << m_quota << "}";

    return ss.str();
}

} // namespace storage
} // namespace norns
