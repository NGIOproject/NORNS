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

#ifndef __IO_TASK_HPP__
#define __IO_TASK_HPP__

#include <cstdint>
#include <memory>

#include "logger.hpp"
#include "common.hpp"
#include "resources.hpp"
#include "backends.hpp"
#include "transferor-registry.hpp"
#include "transferors.hpp"
#include "auth.hpp"
#include "task-stats.hpp"
#include "task-info.hpp"

namespace norns {
namespace io {

// forward declaration
struct transferor;

/*! Descriptor for an I/O task */
template <iotask_type TaskType>
struct task {

    using task_info_ptr = std::shared_ptr<task_info>;
    using transferor_ptr = std::shared_ptr<transferor>;

    task(const task_info_ptr&& task_info)
        : m_task_info(std::move(task_info)) { }

    task(const task_info_ptr&& task_info, const transferor_ptr&& tx_ptr)
        : m_task_info(std::move(task_info)),
          m_transferor(std::move(tx_ptr)) { }

    void operator()();

    const task_info_ptr m_task_info;
    const transferor_ptr m_transferor;
};

/////////////////////////////////////////////////////////////////////////////////
//   specializations for noop tasks 
/////////////////////////////////////////////////////////////////////////////////
template <>
struct task<iotask_type::noop> {

    using task_info_ptr = std::shared_ptr<task_info>;

    task(const task_info_ptr&& task_info, uint32_t sleep_duration)
        : m_task_info(std::move(task_info)),
          m_sleep_duration(sleep_duration) { }

    void operator()() {
        const auto tid = m_task_info->id();

        LOGGER_WARN("[{}] Starting noop I/O task", tid);

        LOGGER_DEBUG("[{}] Sleep for {} usecs", tid, m_sleep_duration);
        usleep(m_sleep_duration);

        m_task_info->update_status(task_status::running);

        LOGGER_WARN("[{}] noop I/O task \"running\"", tid);

        LOGGER_DEBUG("[{}] Sleep for {} usecs", tid, m_sleep_duration);
        usleep(m_sleep_duration);

        m_task_info->update_status(task_status::finished);

        LOGGER_WARN("[{}] noop I/O task completed successfully", tid);
    }

    const task_info_ptr m_task_info;
    const uint32_t m_sleep_duration;
};


} // namespace io
} // namespace norns

#endif // __IO_TASK_HPP__
