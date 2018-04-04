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

#include "common.hpp"
#include "resources.hpp"
#include "backends.hpp"
#include "transferor-registry.hpp"

namespace norns {
namespace io {

using TransferorFunctionType = transferor_registry::CallableType;

// forward declaration
struct task_stats;

/*! Descriptor for an I/O task */
struct task {

    using backend_ptr = std::shared_ptr<storage::backend>;
    using resource_info_ptr = std::shared_ptr<data::resource_info>;
    using resource_ptr = std::shared_ptr<data::resource>;
    using task_stats_ptr = std::shared_ptr<task_stats>;

    task(const iotask_id tid, const iotask_type type, 
         const backend_ptr srd_backend, const resource_info_ptr src_info,
         const backend_ptr dst_backend, const resource_info_ptr dst_info,
         const TransferorFunctionType& cfun, const task_stats_ptr stats);

    iotask_id id() const;
    bool is_valid() const;
    void operator()() const;

    //XXX a munge credential might be better here
    const iotask_id   m_id;
    pid_t       m_pid;
    uint32_t    m_jobid;
    
    const iotask_type m_type;
    const backend_ptr m_src_backend;
    const resource_info_ptr m_src_info;
    const backend_ptr m_dst_backend;
    const resource_info_ptr m_dst_info;
    const TransferorFunctionType m_transferor;

    resource_ptr m_src;
    resource_ptr m_dst;

    const task_stats_ptr m_stats;
};


} // namespace io
} // namespace norns

#endif // __IO_TASK_HPP__
