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

#ifndef __IO_TASK_HPP__
#define __IO_TASK_HPP__

#include <cstdint>
#include <memory>

#include "norns.h"
#include "resources.hpp"
#include "backends.hpp"

namespace norns {
namespace io {

/*! Valid types for an I/O task */
enum class task_type {
    copy,
    move,
    unknown
};

struct task_stats;

/*! Descriptor for an I/O task */
struct task {

    using backend_ptr = std::shared_ptr<storage::backend>;
    using resource_ptr = std::shared_ptr<data::resource>;
    using task_stats_ptr = std::shared_ptr<task_stats>;

    task(norns_tid_t tid, norns_op_t type, const resource_ptr src, 
         const resource_ptr dst, const task_stats_ptr stats);
    norns_tid_t id() const;
    bool is_valid() const;
    void operator()() const;

    static norns_tid_t create_id(); 

    //XXX a munge credential might be better here
    uint64_t    m_id;
    pid_t       m_pid;
    uint32_t    m_jobid;
    
    task_type m_type;
    resource_ptr m_src;
    resource_ptr m_dst;
    task_stats_ptr m_stats;
};


} // namespace io
} // namespace norns

#endif // __IO_TASK_HPP__
