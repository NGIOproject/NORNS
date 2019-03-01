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

#ifndef __IO_TASK_COPY_HPP__
#define __IO_TASK_COPY_HPP__

namespace norns {
namespace io {

/////////////////////////////////////////////////////////////////////////////////
//   specializations for copy tasks 
/////////////////////////////////////////////////////////////////////////////////
template<>
inline void 
task<iotask_type::copy>::operator()() {

    const auto tid = m_task_info->id();
    const auto type = m_task_info->type();
    const auto auth = m_task_info->auth();
    const auto src_backend = m_task_info->src_backend();
    const auto src_rinfo = m_task_info->src_rinfo();
    const auto dst_backend = m_task_info->dst_backend();
    const auto dst_rinfo = m_task_info->dst_rinfo();
    std::error_code ec;

    // helper lambda for error reporting 
    const auto log_error = [&] (const std::string& msg) {

        m_task_info->update_status(task_status::finished_with_error,
                                   urd_error::system_error, ec);
        std::string r_msg = "[{}] " + msg + ": {}";

        LOGGER_ERROR(r_msg.c_str(), tid, ec.message());
        LOGGER_WARN("[{}] I/O task completed with error", tid);

    };

    LOGGER_WARN("[{}] Starting I/O task", tid);
    LOGGER_WARN("[{}]   TYPE: {}", tid, utils::to_string(type));
    LOGGER_WARN("[{}]   FROM: {}", tid, src_backend->to_string());
    LOGGER_WARN("[{}]     TO: {}", tid, dst_backend->to_string());

    m_task_info->update_status(task_status::running);

    auto src = src_backend->get_resource(src_rinfo, ec);

    if(ec) {
        log_error("Could not access input data " + src_rinfo->to_string());
        return;
    }

    auto dst = dst_backend->new_resource(dst_rinfo, src->is_collection(), ec);

    if(ec) {
        log_error("Could not create output data " + dst_rinfo->to_string());
        return;
    }

    ec = m_transferor->transfer(auth, m_task_info, src, dst);

    if(ec) {
        log_error("Transfer failed");
        return;
    }

    LOGGER_WARN("[{}] I/O task completed successfully [{} MiB/s]", 
                tid, m_task_info->bandwidth());

    m_task_info->update_status(task_status::finished, urd_error::success, 
                    std::make_error_code(static_cast<std::errc>(ec.value())));
}

} // namespace io
} // namespace norns

#endif // __IO_TASK_COPY_HPP__
