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

#ifndef __IO_TASK_STATS_HPP__
#define __IO_TASK_STATS_HPP__

#include <string>
#include <system_error>
#include <boost/thread/shared_mutex.hpp>

namespace norns {

enum class urd_error;

namespace io {

/*! Valid status for an I/O task */
enum class task_status {
    undefined,
    pending,
    in_progress,
    finished,
    finished_with_error,
};

/*! Stats about a registered I/O task */
struct task_stats {

    task_stats();
    explicit task_stats(task_status status);
    task_stats(const task_stats& other);
    task_stats(task_stats&& rhs) noexcept;

    task_stats& operator=(const task_stats& other);
    task_stats& operator=(task_stats&& rhs) noexcept;

    task_status status() const;
    void set_status(const task_status status);
    urd_error error() const;
    void set_error(const urd_error ec);
    std::error_code sys_error() const;
    void set_sys_error(const std::error_code& ec);

    mutable boost::shared_mutex m_mutex;
    task_status m_status;
    urd_error m_task_error;
    std::error_code m_sys_error;
};

struct task_stats_view {

    task_stats_view();
    explicit task_stats_view(const task_stats& stats);
    task_stats_view(const task_stats_view& other);
    task_stats_view(task_stats_view&& rhs) noexcept;

    task_stats_view& operator=(const task_stats_view& other);
    task_stats_view& operator=(task_stats_view&& rhs) noexcept;

    task_status status() const;
    urd_error error() const;
    std::error_code sys_error() const;

    task_stats m_stats;
};

} // namespace io

namespace utils {

std::string to_string(io::task_status st);

}

} // namespace norns

#endif /* __IO_TASK_STATS_HPP__ */
