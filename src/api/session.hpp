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

#ifndef __API_SESSION_HPP__
#define __API_SESSION_HPP__

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "common.hpp"

namespace ba = boost::asio;
namespace bfs = boost::filesystem;

namespace norns {
namespace api {

// forward declarations
template <typename Message> class local_endpoint;
template <typename Message> class remote_endpoint;


/* helper class for managing communication sessions with a client */
template <typename Message>
class session : public std::enable_shared_from_this<session<Message>> {

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
    session(ba::generic::stream_protocol::socket&& socket, 
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

        auto self(std::enable_shared_from_this<session<Message>>::shared_from_this());

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

        auto self(std::enable_shared_from_this<session<Message>>::shared_from_this());

        std::size_t body_length = m_message.expected_length(Message::body);

        if(body_length != 0) {
            ba::async_read(m_socket,
                    ba::buffer(m_message.buffer(Message::body), body_length),
                    [this, self](boost::system::error_code ec, std::size_t length) {

                        if(!ec) {
                            Input req = m_message.decode_body(length);

                            Output resp = m_dispatcher->run(req->type(), std::move(req));

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

        auto self(std::enable_shared_from_this<session<Message>>::shared_from_this());

        ba::async_write(m_socket, buffers,
            [this, self](boost::system::error_code ec, std::size_t /*length*/){

//                std::cerr << "Writing done!\n";

                if(!ec){
                    //do_read_request();
                }
            });
    }

    ba::generic::stream_protocol::socket m_socket;
    Message                              m_message;
    std::shared_ptr<Dispatcher> m_dispatcher;
};


} // namespace api
} // namespace norns

#endif /* __API_SESSION_HPP__ */
