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

#ifndef __IPC_LISTENER_HPP__
#define __IPC_LISTENER_HPP__

#include <boost/asio.hpp>
#include <norns.h>
#include <norns-rpc.h>

namespace ba = boost::asio;

template <typename Input, typename Output>
using callback_fn = std::function<std::shared_ptr<Output>(const std::shared_ptr<Input>)>;


/* helper class for managing communication sessions with a client */
template <typename Message, typename Input, typename Output>
class session : public std::enable_shared_from_this<session<Message, Input, Output>> {

public:
    session(ba::local::stream_protocol::socket socket, callback_fn<Input, Output> callback)
        : m_socket(std::move(socket)),
          m_callback(callback) {}

    ~session() {
        //std::cerr << "session dying\n";
    }

    void start(){
        do_read_request();
    }

private:
    void do_read_request(){

        auto self(std::enable_shared_from_this<session<Message, Input, Output>>::shared_from_this());

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

        auto self(std::enable_shared_from_this<session<Message, Input, Output>>::shared_from_this());

        std::size_t body_length = m_message.expected_length(Message::body);

        if(body_length != 0) {
            ba::async_read(m_socket,
                    ba::buffer(m_message.buffer(Message::body), body_length),
                    [this, self](boost::system::error_code ec, std::size_t length) {

                        if(!ec) {
                            std::shared_ptr<Input> req;

                            if(m_message.decode_body(length, req)) {

                                std::shared_ptr<Output> resp = m_callback(req);

                                assert(resp != nullptr);

                                m_message.clear();

//                                return;

                                if(m_message.encode_response(resp)) {
                                    do_write_response();
                                }
                            }
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

        auto self(std::enable_shared_from_this<session<Message, Input, Output>>::shared_from_this());

        ba::async_write(m_socket, buffers,
            [this, self](boost::system::error_code ec, std::size_t /*length*/){

                //std::cerr << "Writing done!\n";

                if(!ec){
                    //do_read_request();
                }
            });
    }

    ba::local::stream_protocol::socket  m_socket;
    callback_fn<Input, Output>          m_callback;
    Message                             m_message;
};


/* simple lister for an AF_UNIX socket that accepts requests asynchronously and
 * invokes a callback with a fixed-length payload */
template <typename Message, typename Input, typename Output>
class ipc_listener {

//    using DataPtr = std::shared_ptr<Input>;

public:
    ipc_listener(const std::string& socket_file, callback_fn<Input, Output> callback) 
        : m_acceptor(m_ios, ba::local::stream_protocol::endpoint(socket_file)),
          m_socket(m_ios),
          m_callback(callback) {
        do_accept();
    }

    void run() {
        m_ios.run();
    }

    void stop() {
        m_ios.stop();
    }

private:
    void do_accept(){
        /* start an asynchronous accept: the call to async_accept returns immediately, 
         * and we use a lambda function as the handler */
        m_acceptor.async_accept(m_socket,
            [this](const boost::system::error_code& ec){
                if(!ec){
                    std::make_shared<session<Message, Input, Output>>(std::move(m_socket), m_callback)->start();
                }

                do_accept();
            });
    }

    boost::asio::io_service                m_ios;
    ba::local::stream_protocol::acceptor   m_acceptor;
    ba::local::stream_protocol::socket     m_socket;
    callback_fn<Input, Output>             m_callback;
};

#endif /* __IPC_LISTENER_HPP__ */
