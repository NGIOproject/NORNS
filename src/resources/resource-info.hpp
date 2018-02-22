/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 *                                                                       *
 * This file is part of the Data Scheduler, a daemon for tracking and    *
 * managing requests for asynchronous data transfer in a hierarchical    *
 * storage environment.                                                  *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The Data Scheduler is free software: you can redistribute it and/or   *
 * modify it under the terms of the GNU Lesser General Public License    *
 * as published by the Free Software Foundation, either version 3 of     *
 * the License, or (at your option) any later version.                   *
 *                                                                       *
 * The Data Scheduler is distributed in the hope that it will be useful, *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with Data Scheduler.  If not, see                *
 * <http://www.gnu.org/licenses/>.                                       *
 *                                                                       *
 *************************************************************************/

#ifndef __RESOURCE_INFO_HPP__
#define __RESOURCE_INFO_HPP__

#include <string>

namespace data {

/*! Supported resource types */
enum class resource_type {
    memory_region,
    local_posix_path,
    shared_posix_path,
    remote_posix_path
};

/*! Base class for data resources */
struct resource_info {
    virtual ~resource_info() {}
    virtual resource_type type() const = 0;
    virtual std::string nsid() const = 0;
    virtual bool is_remote() const = 0;
    virtual std::string to_string() const = 0;
};

} // namespace data

#endif /* __RESOURCE_INFO_HPP__ */
