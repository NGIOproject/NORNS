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

#include <cmath>
#include <limits>
#include "task-stats.hpp"
#include "task-info.hpp"
#include "logger.hpp"

namespace norns {
namespace io {

task_info::task_info(const iotask_id tid, const iotask_type type,
            const auth::credentials& auth,
            const backend_ptr src_backend, const resource_info_ptr src_rinfo,
            const backend_ptr dst_backend, const resource_info_ptr dst_rinfo) :
    m_id(tid),
    m_type(type),
    m_auth(auth),
    m_src_backend(src_backend),
    m_src_rinfo(src_rinfo),
    m_dst_backend(dst_backend),
    m_dst_rinfo(dst_rinfo),
    m_status(task_status::pending),
    m_task_error(urd_error::success),
    m_sys_error(),
    m_bandwidth(std::numeric_limits<double>::quiet_NaN()),
    m_sent_bytes(),
    m_total_bytes() {

    std::error_code ec;
    m_total_bytes = src_backend->get_size(src_rinfo, ec);

    // make sure that m_total_bytes is 0 if an error occurred
    if(ec) {
        m_total_bytes = 0;
    }
}

iotask_id
task_info::id() const {
    return m_id;
}

iotask_type
task_info::type() const {
    return m_type;
}

auth::credentials 
task_info::auth() const {
    return m_auth;
}

task_info::backend_ptr 
task_info::src_backend() const {
    return m_src_backend;
}

task_info::resource_info_ptr 
task_info::src_rinfo() const {
    return m_src_rinfo;
}

task_info::backend_ptr 
task_info::dst_backend() const {
    return m_dst_backend;
}

task_info::resource_info_ptr 
task_info::dst_rinfo() const {
    return m_dst_rinfo;
}

task_status
task_info::status() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return m_status;
}

void
task_info::update_status(const task_status st) {
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    m_status = st;
}

void 
task_info::update_status(const task_status st, const urd_error ec, 
                         const std::error_code& sc) {

    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    m_status = st;
    m_task_error = ec;
    m_sys_error = sc;
}

std::size_t
task_info::sent_bytes() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return m_sent_bytes;
}

std::size_t
task_info::total_bytes() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return m_total_bytes;
}

task_stats 
task_info::stats() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return task_stats(m_status, m_task_error, m_sys_error,
                      m_total_bytes, m_total_bytes - m_sent_bytes);
}

double
task_info::bandwidth() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return m_bandwidth;
}

void
task_info::update_bandwidth(std::size_t bytes, double usecs) {
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    m_bandwidth = (static_cast<double>(bytes)/(1024*1024) / (usecs/1e6));

    LOGGER_DEBUG("[{}] {}({}, {}) => {}", m_id, __FUNCTION__, bytes, usecs, m_bandwidth);
}

void
task_info::record_transfer(std::size_t bytes, double usecs) {
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    m_sent_bytes += bytes;
    m_bandwidth = (static_cast<double>(bytes)/(1024*1024) / (usecs/1e6));

    LOGGER_DEBUG("[{}] {}({}, {}) => {}", m_id, __FUNCTION__, bytes, usecs, m_bandwidth);
}

boost::shared_lock<boost::shared_mutex>
task_info::lock_shared() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return lock;
}

boost::unique_lock<boost::shared_mutex>
task_info::lock_unique() const {
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    return lock;
}

} // namespace io
} // namespace norns
