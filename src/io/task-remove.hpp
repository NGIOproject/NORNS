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

#ifndef __IO_TASK_REMOVE_HPP__
#define __IO_TASK_REMOVE_HPP__

namespace norns {
namespace io {

/////////////////////////////////////////////////////////////////////////////////
//   specializations for remove tasks 
/////////////////////////////////////////////////////////////////////////////////
template<>
inline void 
task<iotask_type::remove>::operator()() {

    std::error_code ec;

    const auto tid = m_task_info->id();
    const auto type = m_task_info->type();
    const auto src_backend = m_task_info->src_backend();
    const auto src_rinfo = m_task_info->src_rinfo();
    //const auto auth = m_task_info->auth();

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

    m_task_info->update_status(task_status::running);

    src_backend->remove(src_rinfo, ec);

    if(ec) {
        log_error("Failed to remove resource " + src_rinfo->to_string());
        return;
    }

    LOGGER_WARN("[{}] I/O task completed successfully", tid);
    m_task_info->update_status(task_status::finished, urd_error::success, 
                    std::make_error_code(static_cast<std::errc>(ec.value())));
}

} // namespace io
} // namespace norns

#endif // __IO_TASK_REMOVE_HPP__
