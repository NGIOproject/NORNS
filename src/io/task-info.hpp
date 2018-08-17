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

#ifndef __TASK_INFO_HPP__
#define __TASK_INFO_HPP__

#include <boost/thread/shared_mutex.hpp>
#include "backends.hpp"
#include "resources.hpp"
#include "auth.hpp"

namespace norns {
namespace io {

// forward declaration
enum class task_status;
struct task_stats;

struct task_info {

    using backend_ptr = std::shared_ptr<storage::backend>;
    using resource_info_ptr = std::shared_ptr<data::resource_info>;

    task_info(const iotask_id tid, const iotask_type type, 
            const auth::credentials& creds,
            const backend_ptr src_backend, const resource_info_ptr src_rinfo,
            const backend_ptr dst_backend, const resource_info_ptr dst_rinfo);

    iotask_id id() const;
    iotask_type type() const;
    auth::credentials auth() const ;
    backend_ptr src_backend() const;
    resource_info_ptr src_rinfo() const;
    backend_ptr dst_backend() const;
    resource_info_ptr dst_rinfo() const;

    task_status status() const;
    void update_status(const task_status st);
    void update_status(const task_status st, const urd_error ec,
                       const std::error_code& sc);

    std::size_t sent_bytes() const;
    std::size_t total_bytes() const;
    double bandwidth() const;
    void update_bandwidth(std::size_t bytes, double usecs);
    void record_transfer(std::size_t bytes, double usecs);

    task_stats stats() const;

    mutable boost::shared_mutex m_mutex;

    // task id and type
    const iotask_id m_id;
    const iotask_type m_type;

    // user credentials
    const auth::credentials m_auth;

    // backends and resources involved in task
    const backend_ptr m_src_backend;
    const resource_info_ptr m_src_rinfo;
    const backend_ptr m_dst_backend;
    const resource_info_ptr m_dst_rinfo;

    // general task status
    task_status m_status;
    urd_error m_task_error;
    std::error_code m_sys_error;

    // some statistics
    double m_bandwidth;
    std::size_t m_sent_bytes;
    std::size_t m_total_bytes;
};

} // namespace io
} // namespace norns
#endif /* __TASK_INFO_HPP__ */
