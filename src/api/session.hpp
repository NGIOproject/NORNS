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

#ifndef __API_SESSION_HPP__
#define __API_SESSION_HPP__

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "common.hpp"
#include "auth/process-credentials.hpp"

namespace ba = boost::asio;
namespace bfs = boost::filesystem;

namespace norns {
namespace api {

// forward declarations
template <typename Message> class local_endpoint;
template <typename Message> class remote_endpoint;


/* helper class for managing communication sessions with a client */
template <typename Socket, typename Message>
class session : public std::enable_shared_from_this<session<Socket, Message>> {

    friend class local_endpoint<Message>;
    friend class remote_endpoint<Message>;

    using Dispatcher = dispatch_table<
        typename Message::key_type, 
        typename Message::key_hash_type, 
        std::unique_ptr<typename Message::response_type>,
        std::unique_ptr<typename Message::request_type>
            >;

    using Input = std::unique_ptr<typename Message::request_type>;
    using Output = std::unique_ptr<typename Message::response_type>;

public:
    session(Socket&& socket, 
            std::shared_ptr<Dispatcher> dispatcher)
        : m_socket(std::move(socket)),
          m_dispatcher(dispatcher) {}

    ~session() {
//        std::cerr << "session dying\n";
    }

    void start() {
        do_read_request();
    }

private:
    void do_read_request() {

        auto self(std::enable_shared_from_this<
                session<Socket, Message>>::shared_from_this());

        std::size_t header_length = m_message.expected_length(Message::header);

        // read the request header and use the information provided in it
        // to read the request body
        ba::async_read(m_socket,
                ba::buffer(m_message.buffer(Message::header), header_length),
                [this, self](boost::system::error_code ec, std::size_t length) {

                    if(!ec && m_message.decode_header(length)) {
                        //FIXME: check what happens if the caller never
                        //sends a body... are we leaking?
                        do_read_request_body();
                    }
                });
    }

    void do_read_request_body() {

        auto self(std::enable_shared_from_this<
                session<Socket, Message>>::shared_from_this());

        std::size_t body_length = m_message.expected_length(Message::body);

        if(body_length != 0) {
            ba::async_read(m_socket,
                    ba::buffer(m_message.buffer(Message::body), body_length),
                    [this, self](boost::system::error_code ec, std::size_t length) {

                        if(!ec) {
                            Input req = m_message.decode_body(length);

                            // if credentials were not provided in the requests payload,
                            // the request probably originated locally. In that case, we
                            // try to get the credentials from the local calling process 
                            // from the connection socket. If this also fails, 
                            // req->credentials() is set to boost::none, and it becomes
                            // the invoked handler's responsibility to validate the request
                            if(!req->credentials()) {
                                req->set_credentials(
                                        auth::credentials::fetch(m_socket));
                            }

                            Output resp = m_dispatcher->invoke(
                                    req->type(), std::move(req));

                            assert(resp != nullptr);

                            m_message.clear();

                            if(m_message.encode_response(std::move(resp))) {
                                do_write_response();
                            }

                            m_socket.shutdown(ba::ip::tcp::socket::shutdown_both, ec);
                        }
                    });
        }
    }

    void do_write_response() {

        std::vector<ba::const_buffer> buffers;
        buffers.push_back(ba::buffer(m_message.buffer(Message::header)));
        buffers.push_back(ba::buffer(m_message.buffer(Message::body)));

        //Message::print_hex(m_message.buffer(Message::header));
        //Message::print_hex(m_message.buffer(Message::body));

        auto self(std::enable_shared_from_this<
                session<Socket, Message>>::shared_from_this());

        ba::async_write(m_socket, buffers,
            [this, self](boost::system::error_code ec, std::size_t /*length*/){

//                std::cerr << "Writing done!\n";

                if(!ec){
                    //do_read_request();
                }
            });
    }

    Socket m_socket;
    Message                              m_message;
    std::shared_ptr<Dispatcher> m_dispatcher;
};


} // namespace api
} // namespace norns

#endif /* __API_SESSION_HPP__ */
