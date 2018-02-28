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

    auto input_stream = data::make_stream(m_src);
    auto output_stream = data::make_stream(m_dst);

    data::buffer b(8192);

    while(input_stream->read(b) != 0) {
        output_stream->write(b);
    }

    LOGGER_WARN("[{}] I/O task complete", m_id);
}

} // namespace io

