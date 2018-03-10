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

#ifndef __IO_TASK_STATS_HPP__
#define __IO_TASK_STATS_HPP__

#include <string>
#include <boost/thread/shared_mutex.hpp>

namespace io {

/*! Valid status for an I/O task */
enum class task_status {
    pending,
    in_progress,
    finished
};

/*! Stats about a registered I/O task */
struct task_stats {

    task_stats(task_status status);
    task_stats(const task_stats& other);
    task_stats(task_stats&& rhs) noexcept;

    task_status status() const;
    void set_status(const task_status status);

    mutable boost::shared_mutex m_mutex;
    task_status m_status;
};

struct task_stats_view {

    task_stats_view(const task_stats& stats);
    task_stats_view(const task_stats_view& other);
    task_stats_view(task_stats_view&& rhs) noexcept;

    task_status status() const;

    task_stats m_stats;
};

} // namespace io

namespace utils {

std::string to_string(io::task_status st);

}

#endif /* __IO_TASK_STATS_HPP__ */
