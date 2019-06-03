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

#ifndef __IO_TASK_HPP__
#define __IO_TASK_HPP__

#include <cstdint>
#include <memory>
#include <boost/variant.hpp>

#include "logger.hpp"
#include "common.hpp"
#include "resources.hpp"
#include "backends.hpp"
#include "transferor-registry.hpp"
#include "transferors.hpp"
#include "auth.hpp"
#include "task-stats.hpp"
#include "task-info.hpp"

namespace norns {
namespace io {

// forward declaration
struct transferor;

/*! Descriptor for an I/O task */
template <iotask_type TaskType>
struct task {

    using task_info_ptr = std::shared_ptr<task_info>;
    using transferor_ptr = std::shared_ptr<transferor>;

    task(const task_info_ptr&& task_info)
        : m_task_info(std::move(task_info)) { }

    task(const task_info_ptr&& task_info, const transferor_ptr&& tx_ptr)
        : m_task_info(std::move(task_info)),
          m_transferor(std::move(tx_ptr)) { }

    task(const task& other) = default;
    task(task&& rhs) = default;
    task& operator=(const task& other) = default;
    task& operator=(task&& rhs) = default;

    void operator()();

    task_info_ptr m_task_info;
    transferor_ptr m_transferor;
};

} // namespace io
} // namespace norns


#include "task-copy.hpp"
#include "task-move.hpp"
#include "task-remove.hpp"
#include "task-remote-transfer.hpp"
#include "task-noop.hpp"
#include "task-unknown.hpp"


namespace norns {
namespace io {

struct generic_task {

    generic_task() :
        m_type(iotask_type::unknown),
        m_impl(task<iotask_type::unknown>()) {}

    template <typename TaskImpl>
    generic_task(iotask_type type, 
                 TaskImpl&& impl) :
        m_type(type),
        m_impl(std::forward<TaskImpl>(impl)) { }

    generic_task(const generic_task& other) = default;
    generic_task(generic_task&& rhs) = default;
    generic_task& operator=(const generic_task& other) = default;
    generic_task& operator=(generic_task&& rhs) = default;

    iotask_id 
    id() const {
        switch(m_type) {
            case iotask_type::noop:
            {
                using TaskType = io::task<iotask_type::noop>;
                return boost::get<TaskType>(m_impl).m_task_info->id();
            }

            case iotask_type::copy:
            {
                using TaskType = io::task<iotask_type::copy>;
                return boost::get<TaskType>(m_impl).m_task_info->id();
            }

            case iotask_type::move:
            {
                using TaskType = io::task<iotask_type::move>;
                return boost::get<TaskType>(m_impl).m_task_info->id();
            }

            case iotask_type::remove:
            {
                using TaskType = io::task<iotask_type::remove>;
                return boost::get<TaskType>(m_impl).m_task_info->id();
            }

            case iotask_type::remote_transfer:
            {
                using TaskType = io::task<iotask_type::remote_transfer>;
                return boost::get<TaskType>(m_impl).m_task_info->id();
            }

            default:
                return static_cast<iotask_id>(0);
        }
    }

    std::shared_ptr<task_info>
    info() const {
        switch(m_type) {
            case iotask_type::noop:
            {
                using TaskType = io::task<iotask_type::noop>;
                return boost::get<TaskType>(m_impl).m_task_info;
            }

            case iotask_type::copy:
            {
                using TaskType = io::task<iotask_type::copy>;
                return boost::get<TaskType>(m_impl).m_task_info;
            }

            case iotask_type::move:
            {
                using TaskType = io::task<iotask_type::move>;
                return boost::get<TaskType>(m_impl).m_task_info;
            }

            case iotask_type::remove:
            {
                using TaskType = io::task<iotask_type::remove>;
                return boost::get<TaskType>(m_impl).m_task_info;
            }

            case iotask_type::remote_transfer:
            {
                using TaskType = io::task<iotask_type::remote_transfer>;
                return boost::get<TaskType>(m_impl).m_task_info;
            }

            default:
                return {};
        }
    }

    void 
    operator()() {
        switch(m_type) {
            case iotask_type::noop:
                boost::get<io::task<iotask_type::noop>>(m_impl)();
                break;
            case iotask_type::copy:
                boost::get<io::task<iotask_type::copy>>(m_impl)();
                break;
            case iotask_type::move:
                boost::get<io::task<iotask_type::move>>(m_impl)();
                break;
            case iotask_type::remove:
                boost::get<io::task<iotask_type::remove>>(m_impl)();
                break;
            case iotask_type::remote_transfer:
                boost::get<io::task<iotask_type::remote_transfer>>(m_impl)();
                break;
            default:
                break;
        }
    }

    iotask_type m_type;
    boost::variant<
        io::task<iotask_type::copy>,
        io::task<iotask_type::move>,
        io::task<iotask_type::remove>,
        io::task<iotask_type::remote_transfer>,
        io::task<iotask_type::noop>,
        io::task<iotask_type::unknown>> m_impl;

};

} // namespace io
} // namespace norns

#endif // __IO_TASK_HPP__
