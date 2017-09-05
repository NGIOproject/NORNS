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
#include "responses.hpp"
#include <iomanip>


// class message implements simple "packing/unpacking" of protocol buffers messages
// produced by libprotobuf-c
class message {

public:

    enum part { header, body };

    static const std::size_t HEADER_LENGTH = NORNS_RPC_HEADER_LENGTH;

    message() :
        m_header(HEADER_LENGTH),
        m_header_length(0),
        m_body_length(0),
        m_expected_body_length(0) { }

    std::size_t expected_length(const message::part part) const {

        switch(part) {
            case header:
                return HEADER_LENGTH;
            case body:
                return m_expected_body_length;
        }

        return 0;
    }

    std::size_t length(const message::part part) const {

        switch(part) {
            case header:
                return m_header_length;
            case body:
                return m_body_length;
        }

        return 0;
    }

    std::vector<uint8_t>& buffer(const message::part part) {
        switch(part) {
            case header:
                return m_header;
            case body:
                return m_body;
        }

        // should never reach here
        assert(false);
        return m_header; // to avoid compiler warning
    }

    void clear() {
        m_header.clear();
        m_body.clear();
        m_header_length = m_body_length = m_expected_body_length = 0;
    }

    bool encode_response(std::shared_ptr<urd_response> resp) {

        if(!resp->store_to_buffer(m_body)) {
            return false;
        }

        m_body_length = m_body.size();

        m_header.resize(HEADER_LENGTH);

        uint64_t* pbuf = (uint64_t*) m_header.data();
        *pbuf = htonll(m_body_length);

        m_header_length = HEADER_LENGTH;

        return true;
    }

    bool decode_header(std::size_t msg_length) {

        if(msg_length != HEADER_LENGTH) {
            return false;
        }

        m_header_length = HEADER_LENGTH;
        m_expected_body_length = ntohll(*((uint64_t*) m_header.data()));

        m_body.resize(m_expected_body_length);

        return true;
    }

    bool decode_body(std::size_t msg_length, std::shared_ptr<urd_request>& decoded_req) {

        //for(size_t i=0; i<m_body.size(); ++i) {
        //    std::cerr << std::setfill('0') << std::setw(2) << std::hex << (int)m_body[i] << " " << std::dec;
        //}
        //std::cerr << "\n";

        decoded_req.reset(urd_request::create_from_buffer(m_body, msg_length));

        // create_from_buffer always returns a valid request, even if a bad request was received
        return true;
    }

    static void print_hex(const std::vector<uint8_t>& buffer) {
        std::cerr << "<< ";
        for(size_t i=0; i<buffer.size(); ++i) {
            std::cerr << std::setfill('0') << std::setw(2) << std::hex << (int)buffer[i] << " " << std::dec;
        }
        std::cerr << " >>\n";
    }


private:
    std::vector<uint8_t>    m_header;
    std::size_t             m_header_length;
    std::vector<uint8_t>    m_body;
    std::size_t             m_body_length;
    std::size_t             m_expected_body_length;
};

#endif /* __MESSAGE_HPP__ */
