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
