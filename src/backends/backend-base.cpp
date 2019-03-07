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

#include <boost/filesystem.hpp>
#include "logger.hpp"
#include "backend-base.hpp"

namespace bfs = boost::filesystem;

namespace norns {
namespace storage {

backend_factory& 
backend_factory::get() {
    static backend_factory _;
    return _;
}

std::shared_ptr<backend>
backend_factory::create(const backend_type type, 
                        const std::string& nsid, 
                        bool track, 
                        const bfs::path& mount, 
                        uint32_t quota) const {

    boost::system::error_code ec;

    bfs::path canonical_mount = bfs::canonical(mount, ec);

    if(ec) {
        LOGGER_ERROR("Invalid mount point: {}", ec.message());
        throw std::invalid_argument("");
    }

    const int32_t id = static_cast<int32_t>(type);

    const auto& it = m_registrar.find(id);

    if(it != m_registrar.end()){
        return std::shared_ptr<backend>(
                it->second(nsid, track, canonical_mount, quota));
    }
    else{
        throw std::invalid_argument("Unrecognized backend type!");
    }
}

} // namespace storage
} // namespace norns
