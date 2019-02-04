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

#include "logger.hpp"
#include "resources.hpp"
#include "auth.hpp"
#include "io/task-info.hpp"
#include "memory-to-remote-path.hpp"

namespace norns {
namespace io {

bool 
memory_region_to_remote_path_transferor::validate(
        const std::shared_ptr<data::resource_info>& src_info,
        const std::shared_ptr<data::resource_info>& dst_info) const {

    const auto& d_src = reinterpret_cast<const data::memory_region_info&>(*src_info);
    const auto& d_dst = reinterpret_cast<const data::remote_path_info&>(*dst_info);

    (void) d_src;
    (void) d_dst;

    LOGGER_WARN("Validation not implemented");

    return true;
}

std::error_code 
memory_region_to_remote_path_transferor::transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<const data::resource>& src,  
        const std::shared_ptr<const data::resource>& dst) const {

    const auto& d_src = reinterpret_cast<const data::memory_region_resource&>(*src);
    const auto& d_dst = reinterpret_cast<const data::remote_path_resource&>(*dst);

    (void) auth;
    (void) task_info;
    (void) d_src;
    (void) d_dst;

    LOGGER_WARN("Transfer not implemented");

    return std::make_error_code(static_cast<std::errc>(0));
}

std::error_code 
memory_region_to_remote_path_transferor::transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<data::resource_info>& src_rinfo,  
        const std::shared_ptr<data::resource_info>& dst_rinfo) const {

    (void) auth;
    (void) task_info;
    (void) src_rinfo;
    (void) dst_rinfo;

    LOGGER_WARN("Transfer not implemented");

    return std::make_error_code(static_cast<std::errc>(0));
}

std::string 
memory_region_to_remote_path_transferor::to_string() const {
    return "transferor[memory_region => remote_path]";
}

} // namespace io
} // namespace norns
