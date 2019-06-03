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

#ifndef __IO_TASK_NOOP_HPP__
#define __IO_TASK_NOOP_HPP__

namespace norns {
namespace io {

/////////////////////////////////////////////////////////////////////////////////
//   specializations for noop tasks 
/////////////////////////////////////////////////////////////////////////////////
template <>
struct task<iotask_type::noop> {

    using task_info_ptr = std::shared_ptr<task_info>;

    task(const task_info_ptr&& task_info, uint32_t sleep_duration)
        : m_task_info(std::move(task_info)),
          m_sleep_duration(sleep_duration) { }

    task(const task& other) = default;
    task(task&& rhs) = default;
    task& operator=(const task& other) = default;
    task& operator=(task&& rhs) = default;

    void operator()() {
        const auto tid = m_task_info->id();

        LOGGER_WARN("[{}] Starting noop I/O task", tid);

        LOGGER_DEBUG("[{}] Sleep for {} usecs", tid, m_sleep_duration);
        usleep(m_sleep_duration);

        m_task_info->update_status(task_status::running);

        LOGGER_WARN("[{}] noop I/O task \"running\"", tid);

        LOGGER_DEBUG("[{}] Sleep for {} usecs", tid, m_sleep_duration);
        usleep(m_sleep_duration);

        m_task_info->update_status(task_status::finished);

        LOGGER_WARN("[{}] noop I/O task completed successfully", tid);
    }

    task_info_ptr m_task_info;
    uint32_t m_sleep_duration;
};

} // namespace io
} // namespace norns

#endif // __IO_TASK_NOOP_HPP__
