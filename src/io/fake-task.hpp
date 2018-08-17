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

#ifndef __IO_FAKE_TASK_HPP__
#define __IO_FAKE_TASK_HPP__

#include <cstdint>
#include <memory>

#include "common.hpp"
#include "resources.hpp"
#include "backends.hpp"
#include "task.hpp"
#include "task-info.hpp"

namespace norns {
namespace io {

// forward declaration
struct task_stats;

/*! Descriptor for a fake I/O task that does nothing but waiting */
struct fake_task : public task {
    fake_task(task_info_ptr&& task_info);
    void operator()() override final;
};


} // namespace io
} // namespace norns

#endif // __IO_FAKE_TASK_HPP__
