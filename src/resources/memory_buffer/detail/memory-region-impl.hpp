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

#ifndef __MEMORY_REGION_IMPL_HPP__
#define __MEMORY_REGION_IMPL_HPP__

#include <string>
#include <memory>

namespace norns {

// forward declarations
namespace storage { namespace detail {
class process_memory;
}}

namespace data {

enum class resource_type;

namespace detail {

template <>
struct resource_impl<resource_type::memory_region> : public resource {

    resource_impl(const std::shared_ptr<const storage::backend> parent,
                  const uint64_t address, const std::size_t size);
    std::string name() const override final;
    resource_type type() const override final;
    bool is_collection() const override final;
    std::size_t packed_size() const override final;
    const std::shared_ptr<const storage::backend> parent() const override final;
    std::string to_string() const override final;

    uint64_t address() const;
    std::size_t size() const;

    const uint64_t m_address;
    const std::size_t m_size;
    const std::shared_ptr<const storage::detail::process_memory> m_parent;
};

} // namespace detail
} // namespace data
} // namespace norns

#endif /* __MEMORY_REGION_IMPL_HPP__ */
