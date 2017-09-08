/* * Copyright (C) 2017 Barcelona Supercomputing Center
 *                    Centro Nacional de Supercomputacion
 *
 * This file is part of the Data Scheduler, a daemon for tracking and managing
 * requests for asynchronous data transfer in a hierarchical storage environment.
 *
 * See AUTHORS file in the top level directory for information
 * regarding developers and contributors.
 *
 * The Data Scheduler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Data Scheduler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Data Scheduler.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DAEMON_COMMUNICATION_H__
#define __DAEMON_COMMUNICATION_H__

#include <norns.h>

#pragma GCC visibility push(hidden)

typedef enum {
    NORNS_SUBMIT_IOTASK,
    NORNS_REGISTER_JOB,
    NORNS_UPDATE_JOB,
    NORNS_UNREGISTER_JOB,
    NORNS_ADD_PROCESS,
    NORNS_REMOVE_PROCESS
} norns_request_t;

int send_transfer_request(struct norns_iotd* iotdp);
int send_job_request(norns_request_t type, struct norns_cred* auth, 
                     uint32_t jobid, struct norns_job* job);
int send_process_request(norns_request_t type, struct norns_cred* auth, 
                         uint32_t jobid, uid_t uid, gid_t gid, pid_t pid);

#pragma GCC visibility pop

#endif /* __DAEMON_COMMUNICATION_H__ */
