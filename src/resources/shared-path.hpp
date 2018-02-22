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

#include "resource-info.hpp"
#include "local-path.hpp"

#ifndef __SHARED_PATH_HPP__
#define __SHARED_PATH_HPP__

namespace data {

/*! Shared filesystem path data */
struct shared_path : public local_path {
    shared_path(std::string nsid, std::string datapath)
        : local_path(nsid, datapath) {}

    resource_type type() const override {
        return resource_type::shared_posix_path;
    }

    std::string to_string() const override {
        return "SHARED_PATH[\"" + m_nsid + "\", \"" + m_datapath + "\"]";
    }
};

} // namespace data

#endif /* __SHARED_PATH_HPP__ */
