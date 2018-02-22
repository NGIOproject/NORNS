/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 *                                                                       *
 * This file is part of the Data Scheduler, a daemon for tracking and    *
 * managing requests for asynchronous data transfer in a hierarchical    *
 * storage environment.                                                  *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The Data Scheduler is free software: you can redistribute it and/or   *
 * modify it under the terms of the GNU Lesser General Public License    *
 * as published by the Free Software Foundation, either version 3 of     *
 * the License, or (at your option) any later version.                   *
 *                                                                       *
 * The Data Scheduler is distributed in the hope that it will be useful, *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with Data Scheduler.  If not, see                *
 * <http://www.gnu.org/licenses/>.                                       *
 *                                                                       *
 *************************************************************************/

#include "messages.pb.h"
#include "response.hpp"

namespace api {

bool response::store_to_buffer(response_ptr response, std::vector<uint8_t>& buffer) {

    norns::rpc::Response rpc_resp;

    rpc_resp.set_status(response->status());

    switch(response->type()) {
        case response_type::transfer_task:
            rpc_resp.set_type(norns::rpc::Response::SUBMIT_IOTASK);
            break;
        case response_type::ping:
            rpc_resp.set_type(norns::rpc::Response::PING);
            break;
        case response_type::job_register: 
            rpc_resp.set_type(norns::rpc::Response::REGISTER_JOB);
            break;
        case response_type::job_update:
            rpc_resp.set_type(norns::rpc::Response::UPDATE_JOB);
            break;
        case response_type::job_unregister:
            rpc_resp.set_type(norns::rpc::Response::UNREGISTER_JOB);
            break;
        case response_type::process_register:
            rpc_resp.set_type(norns::rpc::Response::ADD_PROCESS);
            break;
        case response_type::process_unregister:
            rpc_resp.set_type(norns::rpc::Response::REMOVE_PROCESS);
            break;
        case response_type::backend_register: 
            rpc_resp.set_type(norns::rpc::Response::REGISTER_BACKEND);
            break;
        case response_type::backend_update:
            rpc_resp.set_type(norns::rpc::Response::UPDATE_BACKEND);
            break;
        case response_type::backend_unregister:
            rpc_resp.set_type(norns::rpc::Response::UNREGISTER_BACKEND);
            break;
        case response_type::bad_request:
            rpc_resp.set_type(norns::rpc::Response::BAD_REQUEST);
            break;
    }

    size_t reserved_size = buffer.size();
    size_t message_size = rpc_resp.ByteSize();
    size_t buffer_size = reserved_size + message_size;

    buffer.resize(buffer_size);

    return rpc_resp.SerializeToArray(&buffer[reserved_size], message_size);
}

void response::cleanup() {
    google::protobuf::ShutdownProtobufLibrary();
}

namespace detail {

template<>
std::string transfer_task_response::to_string() const {
    norns_tid_t id = this->get<0>();
    return utils::strerror(m_status) + (id != 0 ? " [tid: " + std::to_string(id) + "]": "");
}


} // namespace detail

} // namespace api
