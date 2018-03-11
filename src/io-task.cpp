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
#include "io-task.hpp"
#include "io-task-stats.hpp"

namespace norns {
namespace io {

task::task(iotask_id tid, iotask_type type, const resource_ptr src, 
           const resource_ptr dst, const task_stats_ptr stats)
    : m_id(tid),
      m_type(type),
      m_src(src),
      m_dst(dst),
      m_stats(stats) { }

iotask_id task::id() const {
    return m_id;
}

iotask_id task::create_id() {
    static std::atomic<iotask_id> base(0);
    return ++base;
}

void task::operator()() const {
    LOGGER_WARN("[{}] Starting I/O task", m_id);
    LOGGER_WARN("[{}]   FROM: {}", m_id, m_src->to_string());
    LOGGER_WARN("[{}]     TO: {}", m_id, m_dst->to_string());

    m_stats->set_status(task_status::in_progress);

    // helper lambda for creating streams and reporting errors
    auto create_stream = [&] (const resource_ptr res, data::stream_type type) 
        -> std::shared_ptr<data::stream> {

        try {
            return data::make_stream(res, type);
        }
        catch(const std::system_error& error) {
            LOGGER_ERROR("[{}] Error creating {} stream for {}: \"{}\"", 
                    m_id, (type == data::stream_type::input ? "input" : "output"),  
                    res->to_string(), error.code().message());
            throw;
        }
    };

    try {
        auto input_stream = create_stream(m_src, data::stream_type::input);
        auto output_stream = create_stream(m_dst, data::stream_type::output);

        //XXX using something like backend->preferred_transfer_size()
        //would be nice
        data::buffer b(8192);
        std::size_t bytes_read = 0;

        while((bytes_read = input_stream->read(b)) != 0) {

            LOGGER_WARN("[{}] {} bytes read from {}", m_id, bytes_read, m_src->to_string());

            std::size_t bytes_written = output_stream->write(b);

            LOGGER_WARN("[{}] {} bytes written to {}", m_id, bytes_written, m_dst->to_string());
        }

        LOGGER_WARN("[{}] I/O task completed successfully", m_id);
    }
    catch(const std::exception& ex) {
        LOGGER_WARN("[{}] I/O task completed with error: {}", m_id, ex.what());
    }

    m_stats->set_status(task_status::finished);
}

} // namespace io
} // namespace norns
