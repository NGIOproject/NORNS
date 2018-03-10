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

#ifndef __REQUESTS_H__
#define __REQUESTS_H__

#pragma GCC visibility push(hidden)

#include "messages.pb-c.h"
#include "norns.h"

typedef enum {
    /* iotasks */
    NORNS_IOTASK_SUBMIT,
    NORNS_IOTASK_STATUS,

    NORNS_PING,

    /* jobs */
    NORNS_JOB_REGISTER,
    NORNS_JOB_UPDATE,
    NORNS_JOB_UNREGISTER,
    /* processes */
    NORNS_PROCESS_ADD,
    NORNS_PROCESS_REMOVE,
    /* backends */
    NORNS_BACKEND_REGISTER,
    NORNS_BACKEND_UPDATE,
    NORNS_BACKEND_UNREGISTER,
    /* other */
    NORNS_BAD_RPC
} norns_rpc_type_t;

typedef struct {
    void* b_data;
    size_t b_size;
} msgbuffer_t;

#define MSGBUFFER_INIT() \
{   .b_data = 0, \
    .b_size = 0 \
}

typedef struct {
    norns_rpc_type_t r_type;
    int r_error_code;
    union {
        size_t r_taskid;
        norns_status_t r_status;
    };
} norns_response_t;

int pack_to_buffer(norns_rpc_type_t type, msgbuffer_t* buf, ...);
int unpack_from_buffer(msgbuffer_t* buf, norns_response_t* response);

#pragma GCC visibility pop

#endif /* __REQUESTS_H__ */
