//
// Copyright (C) 2017 Barcelona Supercomputing Center
//                    Centro Nacional de Supercomputacion
//
// This file is part of the Data Scheduler, a daemon for tracking and managing
// requests for asynchronous data transfer in a hierarchical storage environment.
//
// See AUTHORS file in the top level directory for information
// regarding developers and contributors.
//
// The Data Scheduler is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Data Scheduler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Data Scheduler.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef __IO_TASK_HPP__
#define __IO_TASK_HPP__

namespace io {

struct task {

    using backend_ptr = std::shared_ptr<storage::backend>;

    task(struct norns_iotd* /*iotdp*/) {
    }

    task() {
    }

    void operator()(int /* thread_id */) const {
        std::cout << "Hello from a task!\n";
    }

    uint64_t    m_id;
    pid_t       m_pid;
    uint32_t    m_jobid;
    backend_ptr m_source;
    backend_ptr m_destination;

};

} // namespace io

#endif // __IO_TASK_HPP__
