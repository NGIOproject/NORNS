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
#include "process-memory.hpp"

namespace norns {
namespace storage {
namespace detail {

process_memory::process_memory() { }

bfs::path process_memory::mount() const {
    return "";
}

uint32_t process_memory::quota() const {
    return 0;
}

backend::resource_ptr 
process_memory::new_resource(const resource_info_ptr& rinfo, 
                             const bool is_collection, 
                             std::error_code& ec) const {
    (void) rinfo;
    (void) is_collection;
    (void) ec;
    return nullptr;
}

backend::resource_ptr 
process_memory::get_resource(const resource_info_ptr& rinfo, 
                             std::error_code& ec) const {

    const auto d_rinfo = std::static_pointer_cast<data::memory_region_info>(rinfo);

    ec = std::make_error_code(static_cast<std::errc>(0));
    return std::make_shared<data::memory_region_resource>(
            shared_from_this(), d_rinfo->address(), d_rinfo->size());
}

bool process_memory::accepts(resource_info_ptr res) const {
    switch(res->type()) {
        case data::resource_type::memory_region:
            return true;
        default:
            return false;
    }
}

std::string process_memory::to_string() const {
    return "PROCESS_MEMORY";
}

} // namespace detail
} // namespace storage
} // namespace norns

