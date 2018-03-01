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

#include "resource.hpp"

#ifndef __MEMORY_BUFFER_HPP__
#define __MEMORY_BUFFER_HPP__

namespace data {

/*! Memory buffer data */
struct memory_buffer : public resource_info {

    memory_buffer(std::string nsid, uint64_t address, std::size_t size);
    ~memory_buffer();
    resource_type type() const override;
    std::string nsid() const override;
    bool is_remote() const override;
    std::string to_string() const override;

    std::string m_nsid;
    uint64_t m_address;
    std::size_t m_size;
};



namespace detail {

template<>
struct resource_impl<resource_type::memory_region> : public resource {

    resource_impl(std::shared_ptr<resource_info> base_info);
    std::string to_string() const override;
    //resource_type type() const override;
    std::shared_ptr<resource_info> info() const override;
    std::shared_ptr<storage::backend> backend() const override;
    void set_backend(const std::shared_ptr<storage::backend> backend) override;

    std::shared_ptr<storage::backend> m_backend;
    std::shared_ptr<memory_buffer> m_resource_info;
};

template <>
struct stream_impl<resource_type::memory_region> : public data::stream {
    stream_impl(std::shared_ptr<resource> resource);
    std::size_t read(buffer& b) override;
    std::size_t write(const buffer& b) override;
};

} // namespace detail

/* typedefs for convenience */
using memory_region_resource = detail::resource_impl<resource_type::memory_region>;
using memory_region_stream = detail::stream_impl<resource_type::memory_region>;

} // namespace data


#endif /* __MEMORY_BUFFER_HPP__ */
