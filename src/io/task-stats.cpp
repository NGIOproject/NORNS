/************************************************************************* 
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of NORNS, a service that allows other programs to   *
 * start, track and manage asynchronous transfers of data resources      *
 * between different storage backends.                                   *
 *                                                                       *
 * See AUTHORS file in the top level directory for information regarding *
 * developers and contributors.                                          *
 *                                                                       *
 * This software was developed as part of the EC H2020 funded project    *
 * NEXTGenIO (Project ID: 671951).                                       *
 *     www.nextgenio.eu                                                  *
 *                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining *
 * a copy of this software and associated documentation files (the       *
 * "Software"), to deal in the Software without restriction, including   *
 * without limitation the rights to use, copy, modify, merge, publish,   *
 * distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to *
 * the following conditions:                                             *
 *                                                                       *
 * The above copyright notice and this permission notice shall be        *
 * included in all copies or substantial portions of the Software.       *
 *                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                 *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS   *
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN    *
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN     *
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE      *
 * SOFTWARE.                                                             *
 *************************************************************************/

#include "common.hpp"
#include "task-stats.hpp"

namespace norns {
namespace io {

task_stats::task_stats() :
    m_status(task_status::undefined),
    m_task_error(urd_error::success),
    m_sys_error(),
    m_total_bytes(),
    m_pending_bytes() { }

task_stats::task_stats(task_status st, urd_error ec, const std::error_code& sc,
            std::size_t total_bytes, std::size_t pending_bytes) :
    m_status(st),
    m_task_error(ec),
    m_sys_error(sc),
    m_total_bytes(total_bytes),
    m_pending_bytes(pending_bytes) { }

std::size_t task_stats::pending_bytes() const {
    return m_pending_bytes;
}

std::size_t task_stats::total_bytes() const {
    return m_total_bytes;
}

task_status task_stats::status() const {
    return m_status;
}

void task_stats::set_status(const task_status status) {
    m_status = status;
}


urd_error task_stats::error() const {
    return m_task_error;
}

void task_stats::set_error(const urd_error ec) {
    m_task_error = ec;
}

std::error_code task_stats::sys_error() const {
    return m_sys_error;
}

void task_stats::set_sys_error(const std::error_code& ec) {
    m_sys_error = ec;
}

global_stats::global_stats() :
    m_running_tasks(0),
    m_pending_tasks(0),
    m_eta(std::numeric_limits<double>::quiet_NaN()) {}

global_stats::global_stats(uint32_t running_tasks, uint32_t pending_tasks, 
                           double eta) :
    m_running_tasks(running_tasks),
    m_pending_tasks(pending_tasks),
    m_eta(eta) {}

uint32_t
global_stats::running_tasks() const {
    return m_running_tasks;
}

uint32_t
global_stats::pending_tasks() const {
    return m_pending_tasks;
}

double
global_stats::eta() const {
    return m_eta;
}

} // namespace io

namespace utils {

std::string to_string(io::task_status st) {
    switch(st) {
        case io::task_status::pending:
            return "NORNS_EPENDING";
        case io::task_status::running:
            return "NORNS_EINPROGRESS";
        case io::task_status::finished:
            return "NORNS_EFINISHED";
        case io::task_status::finished_with_error:
            return "NORNS_EFINISHEDWERROR";
        default:
            return "unknown!";
    }
}

std::string to_string(const io::global_stats& gst) {
    return "(r: " + std::to_string(gst.running_tasks()) + 
           ", p:" + std::to_string(gst.pending_tasks()) + 
           ", eta: " + std::to_string(gst.eta()) + ")";
}


} // namespace utils
} // namespace norns
