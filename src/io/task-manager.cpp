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

#include <boost/optional.hpp>

#include "task-manager.hpp"
#include "task-stats.hpp"
#include "common.hpp"

namespace norns {
namespace io {

task_manager::task_manager() {}

boost::optional<task_manager::ReturnType>
task_manager::create() {

    iotask_id tid = ++m_id_base;

    if(m_tasks.count(tid) != 0) {
        --m_id_base;
        return boost::optional<ReturnType>();
    }

    auto it = m_tasks.emplace(tid, 
            std::make_shared<task_stats>(task_status::pending));

    return boost::optional<ReturnType>(
            std::make_tuple(tid, it.first->second));
}

std::shared_ptr<task_stats>
task_manager::find(iotask_id tid) const {

    std::shared_ptr<task_stats> stats_ptr;

    const auto& it = m_tasks.find(tid);

    if(it != m_tasks.end()) {
        stats_ptr = it->second;
    }

    return stats_ptr;
}

} // namespace io
} // namespace norns
