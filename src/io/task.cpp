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

#include <system_error>
#include <atomic>

#include "logger.hpp"
#include "resources.hpp"
#include "transferors.hpp"
#include "task.hpp"
#include "task-stats.hpp"

namespace norns {
namespace io {

task::task(const iotask_id tid, const iotask_type type, 
           const backend_ptr src_backend, const resource_info_ptr src_info,
           const backend_ptr dst_backend, const resource_info_ptr dst_info,
           const TransferorFunctionType& cfun, const task_stats_ptr stats)
    : m_id(tid),
      m_type(type),
      m_src_backend(src_backend),
      m_src_info(src_info),
      m_dst_backend(dst_backend),
      m_dst_info(dst_info),
      m_transferor(cfun),
      m_stats(stats) { }

iotask_id task::id() const {
    return m_id;
}

void task::operator()() const {

    std::error_code ec;

    // helper lambda for error reporting 
    const auto log_error = [&] (const std::string& msg) {
        m_stats->set_status(task_status::finished_with_error);
        m_stats->set_error(urd_error::system_error);
        m_stats->set_sys_error(ec);

        std::string r_msg = "[{}] " + msg + ": {}";

        LOGGER_ERROR(r_msg.c_str(), m_id, ec.message());
        LOGGER_WARN("[{}] I/O task completed with error", m_id);
    };

    LOGGER_WARN("[{}] Starting I/O task", m_id);
    LOGGER_WARN("[{}]   TYPE: {}", m_id, utils::to_string(m_type));
    LOGGER_WARN("[{}]   FROM: {}", m_id, m_src_backend->to_string());
    LOGGER_WARN("[{}]     TO: {}", m_id, m_dst_backend->to_string());

    m_stats->set_status(task_status::in_progress);

    auto src = m_src_backend->get_resource(m_src_info, ec);

    if(ec) {
        log_error("Could not access input data " + m_src_info->to_string());
        return;
    }

    auto dst = m_dst_backend->new_resource(m_dst_info, src->is_collection(), ec);

    if(ec) {
        log_error("Could not create output data " + m_dst_info->to_string());
        return;
    }

    //TODO progress reporting
    ec = m_transferor(src, dst);

    //XXX should we rollback all previous changes?
    if(ec) {
        log_error("Conversion failed");
        return;
    }

    LOGGER_WARN("[{}] I/O task completed successfully", m_id);

    m_stats->set_status(task_status::finished);
    m_stats->set_error(urd_error::success);
    m_stats->set_sys_error(std::make_error_code(static_cast<std::errc>(ec.value())));
}

} // namespace io
} // namespace norns
