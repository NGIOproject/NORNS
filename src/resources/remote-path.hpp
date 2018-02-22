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

#ifndef __REMOTE_PATH_HPP__
#define __REMOTE_PATH_HPP__

namespace data {

/*! Remote filesystem path data */
struct remote_path : public resource_info {

    remote_path(std::string nsid, std::string hostname, std::string datapath)
        : m_nsid(nsid),
          m_hostname(hostname),
          m_datapath(datapath) {}

    resource_type type() const override {
        return resource_type::remote_posix_path;
    }

    std::string nsid() const override {
        return m_nsid;
    }

    bool is_remote() const override {
        return true;
    }

    std::string to_string() const override {
        return "REMOTE_PATH[\"" + m_hostname + "\", \"" + m_nsid + "\", \"" + m_datapath + "\"]";
    }

    std::string m_nsid;
    std::string m_hostname;
    std::string m_datapath;
};

} // namespace data

#endif /* __REMOTE_PATH_HPP__ */
