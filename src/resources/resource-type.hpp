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

#ifndef __RESOURCE_TYPE_HPP__
#define __RESOURCE_TYPE_HPP__

#include <string>

namespace norns {
namespace data {

/*! Supported resource types */
enum class resource_type {
    memory_region,
    local_posix_path,
    shared_posix_path,
    remote_posix_path
};

} // namespace data

namespace utils {

static inline std::string to_string(data::resource_type type) {
    switch(type) {
        case data::resource_type::memory_region:
            return "MEMORY_REGION";
        case data::resource_type::local_posix_path:
            return "LOCAL_PATH";
        case data::resource_type::shared_posix_path:
            return "SHARED_PATH";
        case data::resource_type::remote_posix_path:
            return "REMOTE_PATH";
        default:
            return "UNKNOWN_RESOURCE_TYPE";
    }
}

}

} // namespace norns

#endif /* __RESOURCE_TYPE_HPP__ */
