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

#ifndef __DAEMON_COMMUNICATION_H__
#define __DAEMON_COMMUNICATION_H__

#include "requests.h"

#pragma GCC visibility push(hidden)

norns_error_t send_submit_request(norns_iotask_t* task);
norns_error_t send_control_command_request(nornsctl_command_t cmd, void* args);
norns_error_t send_status_request(norns_iotask_t* task, norns_stat_t* stats);
norns_error_t send_job_request(norns_msgtype_t type, uint32_t jobid, 
                               nornsctl_job_t* job);
norns_error_t send_process_request(norns_msgtype_t type, uint32_t jobid, 
                                   uid_t uid, gid_t gid, pid_t pid);
norns_error_t send_namespace_request(norns_msgtype_t type, const char* nsid, 
                                     nornsctl_backend_t* backend);
norns_error_t send_control_status_request(nornsctl_stat_t* stats);

#pragma GCC visibility pop

#endif /* __DAEMON_COMMUNICATION_H__ */
