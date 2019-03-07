/*************************************************************************
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
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

#ifndef __REMOTE_RESOURCE_INFO_HPP__
#define __REMOTE_RESOURCE_INFO_HPP__

#include <hermes.hpp>
#include <string>
#include "resource-info.hpp"
#include "rpcs.hpp"

namespace norns {
namespace data {

enum class resource_type;

namespace detail {

/*! Local filesystem path data */
struct remote_resource_info : public resource_info {

    remote_resource_info(const std::string& address,
                         const std::string& nsid, 
                         const std::string& name);

    remote_resource_info(const std::string& address,
                         const std::string& nsid, 
                         bool is_collection,
                         const std::string& name,
                         const hermes::exposed_memory& buffers);
    ~remote_resource_info();
    resource_type type() const override final;
    std::string nsid() const override final;
    bool is_remote() const override final;
    std::string to_string() const override final;

    std::string
    address() const;

    std::string
    name() const;

    bool
    is_collection() const;

    bool
    has_buffers() const;

    hermes::exposed_memory
    buffers() const;

    std::string m_address;
    std::string m_nsid;
    bool m_is_collection;
    std::string m_name;
    hermes::exposed_memory m_buffers;
};

} // namespace detail
} // namespace data
} // namespace norns

#endif // __REMOTE_RESOURCE_INFO_HPP__
