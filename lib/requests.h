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

    /* control commands */
    NORNSCTL_COMMAND,

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
