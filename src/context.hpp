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

#ifndef NORNS_CONTEXT_HPP
#define NORNS_CONTEXT_HPP

#include <boost/filesystem.hpp>
#include <memory>

namespace bfs = boost::filesystem;

namespace hermes {
    class async_engine;
} // namespace hermes

namespace norns {

struct context {

    context(bfs::path staging_directory,
            std::shared_ptr<hermes::async_engine> network_service) :
        m_staging_directory(std::move(staging_directory)),
        m_network_service(std::move(network_service)) { }

    bfs::path 
    staging_directory() const {
        return m_staging_directory;
    }

    std::shared_ptr<hermes::async_engine>
    network_service() const {
        return m_network_service;
    }

    bfs::path m_staging_directory;
    std::shared_ptr<hermes::async_engine> m_network_service;
};

} // namespace norns

#endif // NORNS_CONTEXT_HPP
