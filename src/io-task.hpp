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

#ifndef __IO_TASK_HPP__
#define __IO_TASK_HPP__

#include <cstdint>
#include <memory>

#include "norns.h"
#include "resources.hpp"
#include "backends.hpp"

namespace io {

/*! Valid types of IO tasks */
enum class task_type {
    copy,
    move,
    unknown
};

struct task {

    using backend_ptr = std::shared_ptr<storage::backend>;
    using resource_ptr = std::shared_ptr<data::resource>;

    task(norns_op_t type, const resource_ptr src, const resource_ptr dst);
    norns_tid_t id() const;
    bool is_valid() const;
    void operator()() const;

    static norns_tid_t create_id();

    uint64_t    m_id;
    pid_t       m_pid;
    uint32_t    m_jobid;
    
    task_type m_type;
    resource_ptr m_src;
    resource_ptr m_dst;
};

} // namespace io

#endif // __IO_TASK_HPP__
