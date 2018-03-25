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

#include "types.hpp"

namespace norns {
namespace utils {

std::string to_string(backend_type type) {
    switch(type) {
        case backend_type::nvml:
            return "NVML";
        case backend_type::lustre:
            return "LUSTRE";
//        case backend_type::process_memory: // deprecated
//            return "PROCESS_MEMORY";
        case backend_type::echofs:
            return "ECHOFS";
        case backend_type::posix_filesystem:
            return "POSIX_FILESYSTEM";
        default:
            return "UNKNOWN_BACKEND";
    }
}

std::string to_string(iotask_type type) {
    switch(type) {
        case iotask_type::copy:
            return "DATA_COPY";
        case iotask_type::move:
            return "DATA_MOVE";
        default:
            return "UNKNOWN_IOTASK";
    }
}

std::string to_string(urd_error ecode) {

    switch(ecode) {
        case urd_error::success:
            return "NORNS_SUCCESS";
        case urd_error::snafu:
            return "NORNS_ESNAFU";
        case urd_error::bad_args:
            return "NORNS_EBADARGS";
        case urd_error::bad_request:
            return "NORNS_EBADREQUEST";
        case urd_error::out_of_memory:
            return "NORNS_ENOMEM";
        case urd_error::not_supported:
            return "NORNS_ENOTSUPPORTED";
        case urd_error::connection_failed:
            return "NORNS_ECONNFAILED";
        case urd_error::rpc_send_failed:
            return "NORNS_ERPCSENDFAILED";
        case urd_error::rpc_recv_failed:
            return "NORNS_ERPCRECVFAILED";
        case urd_error::job_exists:
            return "NORNS_EJOBEXISTS";
        case urd_error::no_such_job:
            return "NORNS_ENOSUCHJOB";
        case urd_error::process_exists:
            return "NORNS_EPROCESSEXISTS";
        case urd_error::no_such_process:
            return "NORNS_ENOSUCHPROCESS";
        case urd_error::backend_exists:
            return "NORNS_EBACKENDEXISTS";
        case urd_error::no_such_backend:
            return "NORNS_ENOSUCHBACKEND";
        case urd_error::task_exists:
            return "NORNS_ETASKEXISTS";
        case urd_error::no_such_task:
            return "NORNS_ENOSUCHTASK";
        case urd_error::too_many_tasks:
            return "NORNS_ETOOMANYTASKS";
        default:
            return "UNKNOWN_ERROR";
    }
}

} // namespace utils
} // namespace norns


