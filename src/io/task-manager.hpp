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

#ifndef __TASK_MANAGER_HPP__
#define __TASK_MANAGER_HPP__

#include <memory>
#include <unordered_map>
#include <boost/optional.hpp>
#include "thread-pool.hpp"

#include "task.hpp"
#include "fake-task.hpp"

#include "common.hpp"

namespace norns {
namespace io {

// forward declarations
enum class task_status;
struct task_stats;

struct task_manager {

    using ReturnType = 
        std::tuple<iotask_id, std::shared_ptr<task_stats>>;

    task_manager(uint32_t nrunners, bool dry_run);

    template <typename... Args>
    boost::optional<iotask_id>
    create(Args&&... args) {

        auto ret = register_task();

        if(!ret) {
            return boost::none;
        }

        iotask_id tid;
        std::shared_ptr<io::task_stats> stats_record;

        std::tie(tid, stats_record) = *ret;

        if(!m_dry_run) {
            m_runners.submit_and_forget(
                    io::task(tid, std::forward<Args>(args)...,
                                std::move(stats_record)));
            
            return tid;
        }

        m_runners.submit_and_forget(
                io::fake_task(tid, std::move(stats_record)));

        return tid;
    }

    std::shared_ptr<task_stats>
    find(iotask_id) const;

    void 
    stop_all_tasks();

private:
    boost::optional<ReturnType> register_task();

private:
    iotask_id m_id_base = 0;
    bool m_dry_run;
    std::unordered_map<iotask_id, std::shared_ptr<task_stats>> m_task_info;
    thread_pool m_runners;

};

} // namespace io
} // namespace norns

#endif /* __TASK_MANAGER_HPP__ */
