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

#ifndef __IO_TASK_STATS_HPP__
#define __IO_TASK_STATS_HPP__

#include <string>
#include <system_error>
#include <limits>

namespace norns {

enum class urd_error;

namespace io {

/*! Valid status for an I/O task */
enum class task_status {
    undefined,
    pending,
    running,
    finished,
    finished_with_error,
};

/*! Stats about a registered I/O task */
struct task_stats {

    task_stats();
    task_stats(task_status st, urd_error ec, const std::error_code& sc, 
               std::size_t total_bytes, std::size_t pending_bytes);

    std::size_t pending_bytes() const;
    std::size_t total_bytes() const;
    task_status status() const;
    void set_status(const task_status status);
    urd_error error() const;
    void set_error(const urd_error ec);
    std::error_code sys_error() const;
    void set_sys_error(const std::error_code& ec);

    task_status m_status;
    urd_error m_task_error;
    std::error_code m_sys_error;
    std::size_t m_total_bytes;
    std::size_t m_pending_bytes;
};

/*! Global stats about all registered I/O tasks */
struct global_stats {
    global_stats();
    global_stats(uint32_t running_tasks, uint32_t pending_tasks, double eta);

    uint32_t running_tasks() const;
    uint32_t pending_tasks() const;
    double eta() const;

    uint32_t m_running_tasks;
    uint32_t m_pending_tasks;
    double m_eta;
};

} // namespace io

namespace utils {

std::string to_string(io::task_status st);
std::string to_string(const io::global_stats& gst);

}

} // namespace norns

#endif /* __IO_TASK_STATS_HPP__ */
