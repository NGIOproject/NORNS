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

#include "common.hpp"
#include "task-stats.hpp"

namespace norns {
namespace io {

task_stats::task_stats() 
    : m_status(task_status::undefined),
      m_total_bytes(0),
      m_pending_bytes(0),
      m_task_error(urd_error::success), 
      m_sys_error(std::make_error_code(static_cast<std::errc>(0))) { }

task_stats::task_stats(task_status status, std::size_t total_bytes) 
    : m_status(status),
      m_total_bytes(total_bytes),
      m_pending_bytes(total_bytes),
      m_task_error(urd_error::success), 
      m_sys_error(std::make_error_code(static_cast<std::errc>(0))) { }

/*! Return a copy of the current stats by locking the instance and copying 
 * its internal data. This is useful to obtain a "view" of the instance
 * in a state where all data is consistent with each other */
task_stats::task_stats(const task_stats& other) {

    boost::shared_lock<boost::shared_mutex> lock(m_mutex);

    m_status = other.m_status;
    m_total_bytes = other.m_total_bytes;
    m_pending_bytes = other.m_pending_bytes;
    m_task_error = other.m_task_error;
    m_sys_error = other.m_sys_error;
}

task_stats::task_stats(task_stats&& rhs) noexcept 
    : m_status(std::move(rhs.m_status)) { }

task_stats& task_stats::operator=(const task_stats& other) {
    if(this != &other) {
        this->m_status = other.m_status;
        this->m_total_bytes = other.m_total_bytes;
        this->m_pending_bytes = other.m_pending_bytes;
        this->m_task_error = other.m_task_error;
        this->m_sys_error = other.m_sys_error;
    }

    return *this;
}

task_stats& task_stats::operator=(task_stats&& rhs) noexcept {
    if(this != &rhs) {
        this->m_status = std::move(rhs.m_status);
        this->m_total_bytes = std::move(rhs.m_total_bytes);
        this->m_pending_bytes = std::move(rhs.m_pending_bytes);
        this->m_task_error = std::move(rhs.m_task_error);
        this->m_sys_error = std::move(rhs.m_sys_error);
    }

    return *this;
}

std::size_t task_stats::pending_bytes() const {
    return m_pending_bytes;
}

std::size_t task_stats::total_bytes() const {
    return m_total_bytes;
}

task_status task_stats::status() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return m_status;
}

void task_stats::set_status(const task_status status) {
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    m_status = status;
}


urd_error task_stats::error() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return m_task_error;
}

void task_stats::set_error(const urd_error ec) {
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    m_task_error = ec;
}

std::error_code task_stats::sys_error() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return m_sys_error;
}

void task_stats::set_sys_error(const std::error_code& ec) {
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    m_sys_error = ec;
}


task_stats_view::task_stats_view() { }

task_stats_view::task_stats_view(const task_stats& stats) 
    : m_stats(stats) { }

task_stats_view& task_stats_view::operator=(const task_stats_view& other) {

    if(this != &other) {
        this->m_stats = other.m_stats;
    }

    return *this;
}

task_stats_view& task_stats_view::operator=(task_stats_view&& rhs) noexcept {

    if(this != &rhs) {
        this->m_stats = std::move(rhs.m_stats);
    }

    return *this;
}

task_stats_view::task_stats_view(const task_stats_view& other)
    : m_stats(other.m_stats) { }

task_stats_view::task_stats_view(task_stats_view&& rhs) noexcept
    : m_stats(std::move(rhs.m_stats)) {}

task_status task_stats_view::status() const {
    return m_stats.status();
}

urd_error task_stats_view::error() const {
    return m_stats.error();
}

std::error_code task_stats_view::sys_error() const {
    return m_stats.sys_error();
}


} // namespace io

namespace utils {

std::string to_string(io::task_status st) {
    switch(st) {
        case io::task_status::pending:
            return "NORNS_EPENDING";
        case io::task_status::in_progress:
            return "NORNS_EINPROGRESS";
        case io::task_status::finished:
            return "NORNS_EFINISHED";
        case io::task_status::finished_with_error:
            return "NORNS_EFINISHEDWERROR";
        default:
            return "unknown!";
    }
}

} // namespace utils
} // namespace norns
