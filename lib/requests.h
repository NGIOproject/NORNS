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

#ifndef __REQUESTS_H__
#define __REQUESTS_H__

#pragma GCC visibility push(hidden)

#include "messages.pb-c.h"
#include "norns.h"

typedef enum {
    /* iotasks */
    NORNS_IOTASK_SUBMIT,
    NORNS_IOTASK_STATUS,

    NORNSCTL_GLOBAL_STATUS,

    NORNS_PING,

    /* jobs */
    NORNS_JOB_REGISTER,
    NORNS_JOB_UPDATE,
    NORNS_JOB_UNREGISTER,
    /* processes */
    NORNS_PROCESS_ADD,
    NORNS_PROCESS_REMOVE,
    /* namespaces */
    NORNS_NAMESPACE_REGISTER,
    NORNS_NAMESPACE_UPDATE,
    NORNS_NAMESPACE_UNREGISTER,
    /* other */
    NORNS_BAD_RPC
} norns_msgtype_t;

typedef struct {
    void* b_data;
    size_t b_size;
} norns_msgbuffer_t;

#define MSGBUFFER_INIT() \
{   .b_data = 0, \
    .b_size = 0 \
}

typedef struct {
    norns_msgtype_t r_type;
    int r_error_code;
    union {
        size_t r_taskid;
        struct {
            norns_status_t r_status;
            norns_error_t r_task_error;
            int r_errno;
        };
        struct {
            uint32_t r_running_tasks;
            uint32_t r_pending_tasks;
            double r_eta;
        };
    };
} norns_response_t;

int pack_to_buffer(norns_msgtype_t type, norns_msgbuffer_t* buf, ...);
int unpack_from_buffer(norns_msgbuffer_t* buf, norns_response_t* response);

#pragma GCC visibility pop

#endif /* __REQUESTS_H__ */
