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

// forward declare storage::backend
namespace storage {
    class backend;
}

namespace data {

/*! Supported resource types */
enum class resource_type {
    memory_region,
    local_posix_path,
    shared_posix_path,
    remote_posix_path
};

/*! Base class for information about data resources */
struct resource_info {
    virtual ~resource_info() {}
    virtual resource_type type() const = 0;
    virtual std::string nsid() const = 0;
    virtual bool is_remote() const = 0;
    virtual std::string to_string() const = 0;
};

/*! A fully qualified resource for which we have a description and the backend
 * that stores it */
struct resource {
    virtual ~resource() {}
//    virtual resource_type type() const = 0;
    virtual std::shared_ptr<resource_info> info() const = 0;
    virtual std::shared_ptr<storage::backend> backend() const = 0;
    virtual void set_backend(const std::shared_ptr<storage::backend> backend) = 0;
    virtual std::string to_string() const = 0;
};

/*! Data buffer with minimum size */
struct buffer : public std::vector<uint8_t> {
    buffer(std::size_t size)
        : std::vector<uint8_t>(size) {}
};


/*! Supported stream types */
enum class stream_type {
    input,
    output
};

/*! A resource stream for reading and/or writing data */
struct stream {
    virtual ~stream() { }

    virtual std::size_t read(buffer& b) = 0;
    virtual std::size_t write(const buffer& b) = 0;
};


namespace detail {

/*! Generic template for resource implementations 
 * (a concrete specialization must be provided for each resource_type) */
template <resource_type RT>
struct resource_impl;

/*! Generic template for resource stream implementations
 * (a concrete specialization must be provided for each resource_type) */
template <resource_type RT>
struct stream_impl;

} // namespace detail
} // namespace data 

#endif /* __RESOURCE_HPP__ */
