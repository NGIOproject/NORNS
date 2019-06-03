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

#ifndef __API_MESSAGE_HPP__
#define __API_MESSAGE_HPP__

#include <norns-rpc.h>
#include <arpa/inet.h>
#include <iomanip>
#include <iostream> // for print_hex, XXX remove if not needed

namespace {

uint64_t htonll(uint64_t x) __attribute__((unused));
uint64_t ntohll(uint64_t x) __attribute__((unused));

uint64_t htonll(uint64_t x) {
    return ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32));
} 

uint64_t ntohll(uint64_t x) {
    return ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32));
}

}

namespace norns {
namespace api {

// class message implements simple "packing/unpacking" of protocol buffers messages
// produced by libprotobuf-c
template <typename Request, typename Response>
class message {

    //using request_ptr = std::shared_ptr<Request>;
    //using response_ptr = std::shared_ptr<Response>;
    using request_ptr = std::unique_ptr<Request>;
    using response_ptr = std::unique_ptr<Response>;

public:

    using request_type = Request;
    using response_type = Response;
    using key_type = typename Request::key_type;
    using key_hash_type = typename Request::key_hash_type;

    enum part { header, body };

    static const std::size_t HEADER_LENGTH = sizeof(uint64_t);

    message() :
        m_header(HEADER_LENGTH),
        m_header_length(0),
        m_body_length(0),
        m_expected_body_length(0) { }

    static void cleanup() {
        Request::cleanup();
        Response::cleanup();
    }

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

    bool encode_response(response_ptr resp) {

        if(!Response::store_to_buffer(std::move(resp), m_body)) {
            return false;
        }

        m_body_length = m_body.size();

        m_header.resize(HEADER_LENGTH);

        uint64_t* pbuf = (uint64_t*) m_header.data();
        *pbuf = ::htonll(m_body_length);

        m_header_length = HEADER_LENGTH;

        return true;
    }

    bool decode_header(std::size_t msg_length) {

        if(msg_length != HEADER_LENGTH) {
            return false;
        }

        m_header_length = HEADER_LENGTH;
        m_expected_body_length = ::ntohll(*((uint64_t*) m_header.data()));

        m_body.resize(m_expected_body_length);

        return true;
    }

    request_ptr decode_body(std::size_t msg_length) {
        return Request::create_from_buffer(m_body, msg_length);
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

} // namespace api
} // namespace norns

#endif /* __API_MESSAGE_HPP__ */
