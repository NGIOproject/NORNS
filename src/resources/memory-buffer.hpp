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

#include <sstream>
#include "resource-info.hpp"

#ifndef __MEMORY_BUFFER_HPP__
#define __MEMORY_BUFFER_HPP__

namespace data {

/*! Memory buffer data */
struct memory_buffer : public resource_info {

    memory_buffer(std::string nsid, uint64_t address, std::size_t size)
        : m_nsid(nsid),
          m_address(address),
          m_size(size) {}

    resource_type type() const override {
        return resource_type::memory_region;
    }

    std::string nsid() const override {
        return m_nsid;
    }

    bool is_remote() const override {
        return false;
    }

    std::string to_string() const override {
        std::stringstream ss;
        ss << "0x" << std::hex << m_address << "+" << "0x" << m_size;
        return "MEMBUF[" + ss.str() + "]";
    }

    std::string m_nsid;
    uint64_t m_address;
    std::size_t m_size;
};

} // namespace data

#endif /* __MEMORY_BUFFER_HPP__ */
