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

#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__

#include <norns-rpc.h>
#include "requests.hpp"


// class message implements simple "packing/unpacking" of protocol buffers messages
// produced by libprotobuf-c
class message {

    static const std::size_t g_max_header_length = NORNS_RPC_HEADER_LENGTH;

public:
    message() :
        m_buffer(g_max_header_length),
        m_header_length(0), 
        m_body_length(0) {}


    std::vector<uint8_t>& buffer() {
        return m_buffer;
    }

    std::size_t max_header_length() const {
        return g_max_header_length;
    }

    std::size_t header_length() const {
        return m_header_length;
    }

    std::size_t body_length() const {
        return m_body_length;
    }

    bool decode_header(std::size_t msg_length) {

        if(msg_length < g_max_header_length) {
            return false;
        }

        m_body_length = ntohll(*((uint64_t*) m_buffer.data()));

        return true;
    }

    bool decode_body(std::size_t msg_length, urd_request*& decoded_req) {

        decoded_req = urd_request::create_from_buffer(&m_buffer[m_header_length], msg_length);

        if(decoded_req != nullptr) {
            return true;
        }

        return false;
    }

private:
    std::vector<uint8_t>    m_buffer;
    std::size_t             m_header_length;
    std::size_t             m_body_length;

};

#endif /* __MESSAGE_HPP__ */
