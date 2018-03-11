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
#include "process-memory.hpp"

namespace norns {
namespace storage {

process_memory::process_memory(const std::string& mount, uint32_t quota) { 
    (void) mount;
    (void) quota;
}

std::string process_memory::mount() const {
    return "";
}

uint32_t process_memory::quota() const {
    return 0;
}

bool process_memory::accepts(resource_info_ptr res) const {
    switch(res->type()) {
        case data::resource_type::memory_region:
            return true;
        default:
            return false;
    }
}

bool process_memory::contains(resource_info_ptr res) const {
    return true; //XXX do actual check
}

void process_memory::read_data() const {
}

void process_memory::write_data() const {
}

std::string process_memory::to_string() const {
    return "PROCESS_MEMORY";
}

} // namespace storage
} // namespace norns
