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
