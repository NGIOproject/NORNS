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

/*************************************************************************
 * Types used internally by the urd daemon.                              *
 *                                                                       *
 * NOTE: Where possible, we reuse the types provided by the NORNS API    *
 *************************************************************************/

#include <string>
#include "norns.h"
#include "nornsctl.h"

#ifndef __URD_TYPES_HPP__
#define __URD_TYPES_HPP__

namespace norns  {

/*! I/O task identifier */
using iotask_id = norns_tid_t;

/*! Valid types for an I/O task */
enum class iotask_type {
    copy,
    move,
    remove,
    remote_transfer,
    noop,
    unknown
};

/*! Backend type */
enum class backend_type {
    nvml             = NORNS_BACKEND_NVML,
    lustre           = NORNS_BACKEND_LUSTRE,
//    process_memory   = NORNS_BACKEND_PROCESS_MEMORY, // deprecated
    echofs           = NORNS_BACKEND_ECHOFS,
    posix_filesystem = NORNS_BACKEND_POSIX_FILESYSTEM,
    unknown
};

enum class command_type {
    ping,
    pause_listen,
    resume_listen,
    shutdown,
    unknown
};

/*! Error codes */
enum class urd_error : norns_error_t {
    success           = NORNS_SUCCESS,

    /* misc errors */
    snafu             = NORNS_ESNAFU,
    bad_args          = NORNS_EBADARGS,
    bad_request       = NORNS_EBADREQUEST,
    out_of_memory     = NORNS_ENOMEM,
    not_supported     = NORNS_ENOTSUPPORTED,
    system_error      = NORNS_ESYSTEMERROR,
    
    /* errors about communication */
    connection_failed = NORNS_ECONNFAILED,
    rpc_send_failed   = NORNS_ERPCSENDFAILED,
    rpc_recv_failed   = NORNS_ERPCRECVFAILED,
    accept_paused     = NORNS_EACCEPTPAUSED,
    
    /* errors about jobs */
    job_exists        = NORNS_EJOBEXISTS,
    no_such_job       = NORNS_ENOSUCHJOB,
    
    /* errors about processes */
    process_exists    = NORNS_EPROCESSEXISTS,
    no_such_process   = NORNS_ENOSUCHPROCESS,
    
    /* errors about backends */
    namespace_exists    = NORNS_ENAMESPACEEXISTS,
    no_such_namespace   = NORNS_ENOSUCHNAMESPACE,
    namespace_not_empty = NORNS_ENAMESPACENOTEMPTY,
    
    /* errors about tasks */
    task_exists       = NORNS_ETASKEXISTS,
    no_such_task      = NORNS_ENOSUCHTASK,
    too_many_tasks    = NORNS_ETOOMANYTASKS,
    tasks_pending     = NORNS_ETASKSPENDING,

    /* errors about resources */
    resource_exists   = NORNS_ERESOURCEEXISTS,
    no_such_resource  = NORNS_ENOSUCHRESOURCE,
};

namespace utils {

std::string to_string(backend_type type);
std::string to_string(iotask_type type);
std::string to_string(urd_error ecode);

}

} // namespace norns

#endif /* __URD_TYPES_HPP__ */
