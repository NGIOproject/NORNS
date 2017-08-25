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

template <typename Data>
using callback_fn = std::function<void(const std::shared_ptr<Data>&)>;


/* helper class for managing communication sessions with a client */
template <typename Message, typename Data>
class session : public std::enable_shared_from_this<session<Message, Data>> {

public:
    session(ba::local::stream_protocol::socket socket, callback_fn<Data> callback)
        : m_socket(std::move(socket)),
          m_callback(callback) {}

    void start(){
        do_read_request_header();
    }

private:
    void do_read_request_header(){

        auto self(std::enable_shared_from_this<session<Message, Data>>::shared_from_this());

        ba::async_read(m_socket,
                ba::buffer(m_message.buffer(), m_message.max_header_length()),
                [this, self](boost::system::error_code ec, std::size_t length){

                    std::cout << "XReceived: " << length << "\n";

                    if(!ec && m_message.decode_header(length)) {
                        //FIXME: check what happens if the caller never
                        //sends a body... are we leaking?
                        do_read_request_body();
                    }
                });
    }

    void do_read_request_body() {

        auto self(std::enable_shared_from_this<session<Message, Data>>::shared_from_this());

        std::size_t header_length = m_message.header_length();
        std::size_t body_length = m_message.body_length();

        if(body_length != 0) {

            std::cout << body_length << "\n";

            std::size_t new_length = header_length + body_length;
            m_message.buffer().resize(new_length);

            ba::async_read(m_socket,
                    ba::buffer(&m_message.buffer()[header_length], body_length),
                    [this, self](boost::system::error_code ec, std::size_t length) {

                        if(!ec) {

                            Data* payload;

                            if(m_message.decode_body(length, payload)) {

                                if(payload != nullptr) {
                                    m_callback(std::shared_ptr<Data>(payload));
                    //            do_write_response(length);
                                }
                            }
                        }
                    });
        }
    }

    void do_write_response(std::size_t length){

        //auto self(std::enable_shared_from_this<session<T>>::shared_from_this());

        //ba::async_write(m_socket, boost::asio::buffer(m_data, length),
        //    [this, self](boost::system::error_code ec, std::size_t /*length*/){
        //        if(!ec){
        //            do_read_request_header(); ??
        //        }
        //    });
    }

    ba::local::stream_protocol::socket  m_socket;
    callback_fn<Data>                   m_callback;
    Message                             m_message;
};


/* simple lister for an AF_UNIX socket that accepts requests asynchronously and
 * invokes a callback with a fixed-length payload */
template <typename Message, typename Data>
class ipc_listener {

//    using DataPtr = std::shared_ptr<Data>;

public:
    ipc_listener(const std::string& socket_file, callback_fn<Data> callback) 
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
                    std::make_shared<session<Message, Data>>(std::move(m_socket), m_callback)->start();
                }

                do_accept();
            });
    }

    boost::asio::io_service                m_ios;
    ba::local::stream_protocol::acceptor   m_acceptor;
    ba::local::stream_protocol::socket     m_socket;
    callback_fn<Data>                   m_callback;
};

#endif /* __IPC_LISTENER_HPP__ */
