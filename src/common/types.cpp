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
        case iotask_type::remove:
            return "DATA_REMOVE";
        case iotask_type::remote_transfer:
            return "DATA_TRANSFER";
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
        case urd_error::system_error:
            return "NORNS_ESYSTEM_ERROR";
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
        case urd_error::namespace_exists:
            return "NORNS_ENAMESPACEEXISTS";
        case urd_error::no_such_namespace:
            return "NORNS_ENOSUCHNAMESPACE";
        case urd_error::namespace_not_empty:
            return "NORNS_ENAMESPACENOTEMPTY";
        case urd_error::task_exists:
            return "NORNS_ETASKEXISTS";
        case urd_error::no_such_task:
            return "NORNS_ENOSUCHTASK";
        case urd_error::too_many_tasks:
            return "NORNS_ETOOMANYTASKS";
        case urd_error::tasks_pending:
            return "NORNS_ETASKSPENDING";
        case urd_error::accept_paused:
            return "NORNS_EACCEPTPAUSED";
        case urd_error::resource_exists:
            return "NORNS_ERESOURCEEXISTS";
        case urd_error::no_such_resource:
            return "NORNS_ENOSUCHRESOURCE";
        default:
            return "UNKNOWN_ERROR";
    }
}

} // namespace utils
} // namespace norns


