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

#ifndef __NVML_DAX_HPP__
#define __NVML_DAX_HPP__

#include <system_error>
#include <boost/filesystem.hpp>

#include "backend-base.hpp"

namespace bfs = boost::filesystem;

namespace norns {
namespace storage {

class nvml_dax final : public storage::backend {
public:
    nvml_dax(const bfs::path& mount, uint32_t quota);

    bfs::path mount() const override;
    uint32_t quota() const override;

    resource_ptr new_resource(const resource_info_ptr& rinfo, bool is_collection, std::error_code& ec) const override final;
    resource_ptr get_resource(const resource_info_ptr& rinfo, std::error_code& ec) const override final;
    void remove(const resource_info_ptr& rinfo, std::error_code& ec) const override final;
    std::size_t get_size(const resource_info_ptr& rinfo, std::error_code& ec) const override final;

    bool accepts(resource_info_ptr res) const override final;
    std::string to_string() const final;

private:
    bfs::path m_mount;
    uint32_t  m_quota;
};

//NORNS_REGISTER_BACKEND(backend_type::nvml, nvml_dax);

} // namespace storage
} // namespace norns


#endif // __NVML_DAX_HPP__
