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

#include "messages.pb.h"
#include "io/task-stats.hpp"
#include "response.hpp"
#include "logger.hpp"

namespace {

norns::rpc::Response_Type encode(norns::api::response_type type) {

    using norns::api::response_type;

    switch(type) {
        case response_type::iotask_create:
            return norns::rpc::Response::IOTASK_SUBMIT;
        case response_type::iotask_status:
            return norns::rpc::Response::IOTASK_STATUS;
        case response_type::ping:
            return norns::rpc::Response::PING;
        case response_type::job_register: 
            return norns::rpc::Response::JOB_REGISTER;
        case response_type::job_update:
            return norns::rpc::Response::JOB_UPDATE;
        case response_type::job_unregister:
            return norns::rpc::Response::JOB_UNREGISTER;
        case response_type::process_register:
            return norns::rpc::Response::PROCESS_ADD;
        case response_type::process_unregister:
            return norns::rpc::Response::PROCESS_REMOVE;
        case response_type::backend_register: 
            return norns::rpc::Response::NAMESPACE_REGISTER;
        case response_type::backend_update:
            return norns::rpc::Response::NAMESPACE_UPDATE;
        case response_type::backend_unregister:
            return norns::rpc::Response::NAMESPACE_UNREGISTER;
        case response_type::bad_request:
            return norns::rpc::Response::BAD_REQUEST;
        default:
            assert(false && "Unexpected response_type");
    }
}

::google::protobuf::uint32 encode(norns::io::task_status status) {

    using norns::io::task_status;

    switch(status) {
        case task_status::pending:
            return NORNS_EPENDING;
        case task_status::in_progress:
            return NORNS_EINPROGRESS;
        case task_status::finished:
            return NORNS_EFINISHED;
        case task_status::finished_with_error:
            return NORNS_EFINISHEDWERROR;
        default:
            assert(false && "Unexpected task_status");
    }
}

constexpr ::google::protobuf::uint32 encode(norns::urd_error ecode) {
    return static_cast<::google::protobuf::uint32>(ecode);
}

} // anonymous namespace

namespace norns {
namespace api {

bool response::store_to_buffer(response_ptr response, std::vector<uint8_t>& buffer) {

    norns::rpc::Response rpc_resp;

    rpc_resp.set_error_code(encode(response->error_code()));
    rpc_resp.set_type(encode(response->type()));
    response->pack_extra_info(rpc_resp);

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

///////////////////////////////////////////////////////////////////////////////
//   specializations for iotask_create_response 
///////////////////////////////////////////////////////////////////////////////
template<>
void iotask_create_response::pack_extra_info(norns::rpc::Response& r) const {
    r.set_taskid(this->get<0>());
}

template<>
std::string iotask_create_response::to_string() const {
    auto id = this->get<0>();
    return utils::to_string(m_error_code) + (id != 0 ? " [tid: " + std::to_string(id) + "]": "");
}

/////////////////////////////////////////////////////////////////////////////////
//   specializations for iotask_status_response 
/////////////////////////////////////////////////////////////////////////////////
template<>
void iotask_status_response::pack_extra_info(norns::rpc::Response& r) const {
    const auto& stats = this->get<0>();

    auto stats_msg = new norns::rpc::Response_TaskStats();

    stats_msg->set_status(encode(stats.status()));
    stats_msg->set_task_error(encode(stats.error()));
    stats_msg->set_sys_errnum(stats.sys_error().value());
    r.set_allocated_stats(stats_msg);

    // we don't need to free stats_msg because 
    // set_allocated_stats() frees it once it has copied the data
}

template<>
std::string iotask_status_response::to_string() const {
    const auto& stats = this->get<0>();
    return utils::to_string(stats.status());
}


} // namespace detail

} // namespace api
} // namespace norns
