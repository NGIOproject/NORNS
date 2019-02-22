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

#ifndef __RESOURCE_HPP__
#define __RESOURCE_HPP__

#include <memory>
#include <vector>

namespace norns {

// forward declare storage::backend
namespace storage {
    class backend;
}

namespace data {

enum class resource_type;

/*! A fully qualified resource for which we have a description and the backend
 * that stores it */
struct resource : public std::enable_shared_from_this<resource> {
    virtual ~resource() {}
    virtual resource_type type() const = 0;
    virtual std::string name() const = 0;
    virtual bool is_collection() const = 0;
    virtual std::size_t packed_size() const = 0;
    virtual const std::shared_ptr<const storage::backend> parent() const = 0; 
    virtual std::string to_string() const = 0;
};

namespace detail {

/*! Generic template for resource implementations 
 * (a concrete specialization must be provided for each resource_type) */
template <resource_type RT>
struct resource_impl;

} // namespace detail
} // namespace data 
} // namespace norns

#endif /* __RESOURCE_HPP__ */
