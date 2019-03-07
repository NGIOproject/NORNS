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

#ifndef __REMOTE_RESOURCE_IMPL_HPP__
#define __REMOTE_RESOURCE_IMPL_HPP__

#include <string>
#include <memory>
#include <vector>
#include <system_error>
#include <hermes.hpp>

namespace norns {

// forward declarations
namespace storage {
namespace detail {
class remote_backend;
}
}

namespace data {

enum class resource_type;

namespace detail {

template <>
struct resource_impl<resource_type::remote_resource> : public resource {

    resource_impl(const std::shared_ptr<const storage::backend> parent, 
                  const std::shared_ptr<const remote_resource_info> rinfo);

    std::string name() const override final;
    resource_type type() const override final;
    bool is_collection() const override final;
    std::size_t packed_size() const override final;
    const std::shared_ptr<const storage::backend> parent() const override final;
    std::string to_string() const override final;

    std::string
    address() const;

    std::string
    nsid() const;

    bool
    has_buffer() const;

    hermes::exposed_memory
    buffers() const;

    const std::string m_name;
    const std::string m_address;
    const std::string m_nsid;
    const hermes::exposed_memory m_buffers;
    const bool m_is_collection;
    const std::shared_ptr<const storage::detail::remote_backend> m_parent;
};

} // namespace detail
} // namespace data
} // namespace norns

#endif /* __REMOTE_RESOURCE_IMPL_HPP__ */
