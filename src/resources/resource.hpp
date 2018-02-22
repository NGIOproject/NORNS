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

#ifndef __RESOURCE_HPP__
#define __RESOURCE_HPP__

#include <memory>
#include "resource-info.hpp"
#include "backends.hpp"

namespace data {

/*! A fully qualified resource for which we have a description and the backend
 * that stores it */
struct resource {

    using backend_ptr = std::shared_ptr<storage::backend>;
    using resource_info_ptr = std::shared_ptr<data::resource_info>;

    resource(backend_ptr backend, resource_info_ptr rinfo) 
        : m_backend(backend),
          m_resource_info(rinfo) { }

    resource_type type() const {
        return m_resource_info->type();
    }

    std::string to_string() const {
        return m_backend->to_string() + m_resource_info->to_string();
    }


    backend_ptr         m_backend;
    resource_info_ptr   m_resource_info;
};

} // namespace data 

#endif /* __RESOURCE_HPP__ */
