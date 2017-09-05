//
// Copyright (C) 2017 Barcelona Supercomputing Center
//                    Centro Nacional de Supercomputacion
//
// This file is part of the Data Scheduler, a daemon for tracking and managing
// requests for asynchronous data transfer in a hierarchical storage environment.
//
// See AUTHORS file in the top level directory for information
// regarding developers and contributors.
//
// The Data Scheduler is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Data Scheduler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Data Scheduler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "messages.pb.h"
#include "responses.hpp"
#include "utils.hpp"
#include <sstream>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/transformed.hpp>

generic_response::generic_response() 
 : m_status(0) {}

generic_response::generic_response(uint32_t status) 
 : m_status(status) { }

void generic_response::set_status(uint32_t status) {
    m_status = status;
}

bool generic_response::store_to_buffer(std::vector<uint8_t>& buffer) {

    norns::rpc::Response rpc_resp;

    rpc_resp.set_status(m_status);

    size_t reserved_size = buffer.size();
    size_t message_size = rpc_resp.ByteSize();
    size_t buffer_size = reserved_size + message_size;

    buffer.resize(buffer_size);

    return rpc_resp.SerializeToArray(&buffer[reserved_size], message_size);
}

std::string generic_response::to_string() const {
    return utils::strerror(m_status);
}

job_registration_response::job_registration_response() 
 : m_status(0) { }

job_registration_response::job_registration_response(uint32_t status) 
 : m_status(status) { }

void job_registration_response::set_status(uint32_t status) {
    m_status = status;
}

bool job_registration_response::store_to_buffer(std::vector<uint8_t>& buffer) {

    norns::rpc::Response rpc_resp;

    rpc_resp.set_status(m_status);

    size_t reserved_size = buffer.size();
    size_t message_size = rpc_resp.ByteSize();
    size_t buffer_size = reserved_size + message_size;

    buffer.resize(buffer_size);

    return rpc_resp.SerializeToArray(&buffer[reserved_size], message_size);
}

std::string job_registration_response::to_string() const {
    return utils::strerror(m_status);
}
