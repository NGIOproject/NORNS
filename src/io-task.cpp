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

#include <system_error>
#include <atomic>

#include "norns.h"
#include "logger.hpp"
#include "resources.hpp"
#include "io-task.hpp"

namespace {

io::task_type remap_type(norns_op_t type) {
    switch(type) {
        case NORNS_IOTASK_COPY:
            return io::task_type::copy;
        case NORNS_IOTASK_MOVE:
            return io::task_type::move;
        default:
            return io::task_type::unknown;
    }
}

}

namespace io {

task::task(norns_op_t type, const resource_ptr src, const resource_ptr dst)
    : m_id(create_id()),
      m_type(remap_type(type)),
      m_src(src),
      m_dst(dst) { }

norns_tid_t task::id() const {
    return m_id;
}

norns_tid_t task::create_id() {
    static std::atomic<norns_tid_t> base(0);
    return ++base;
}

void task::operator()() const {
    LOGGER_WARN("[{}] Starting I/O task", m_id);
    LOGGER_WARN("[{}]   FROM: {}", m_id, m_src->to_string());
    LOGGER_WARN("[{}]     TO: {}", m_id, m_dst->to_string());

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
        LOGGER_WARN("[{}] I/O task completed with error", m_id);
    }
}

} // namespace io

