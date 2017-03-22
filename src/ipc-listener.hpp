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

namespace ba = boost::asio;

/* simple lister for an AF_UNIX socket that accepts requests asynchronously and
 * invokes a callback with a fixed-length payload */
template <typename Payload>
class ipc_listener {

    typedef std::function<void(Payload*)> callback_t;

    /* helper class for managing communication sessions with a client */
    template <typename T>
    class session : public std::enable_shared_from_this<session<T>> {

        static const int32_t max_buffer_length = sizeof(T);

    public:
        session(ba::local::stream_protocol::socket socket, callback_t callback)
            : m_socket(std::move(socket)),
              m_callback(callback) {}

        void start(){
            do_read();
        }

    private:
        void do_read(){

            auto self(std::enable_shared_from_this<session<T>>::shared_from_this());

            m_socket.async_read_some(boost::asio::buffer(m_data, max_buffer_length),
                [this, self](boost::system::error_code ec, std::size_t length){
                    if(!ec){
                        T* payload = (T*) &m_data;
                        m_callback(payload);
                        do_write(length);
                    }
                });
        }

        void do_write(std::size_t length){

            auto self(std::enable_shared_from_this<session<T>>::shared_from_this());

            ba::async_write(m_socket, boost::asio::buffer(m_data, length),
                [this, self](boost::system::error_code ec, std::size_t /*length*/){
                    if(!ec){
                        do_read();
                    }
                });
        }

        ba::local::stream_protocol::socket m_socket;
        callback_t                         m_callback;
        char                               m_data[max_buffer_length];
    };

public:
    ipc_listener(const std::string& socket_file, callback_t callback) 
        : m_acceptor(m_ios, ba::local::stream_protocol::endpoint(socket_file)),
          m_socket(m_ios),
          m_callback(callback) {
        start_accept();
    }

    void run() {
        m_ios.run();
    }

    void stop() {
        m_ios.stop();
    }

private:
    void start_accept(){
        /* start an asynchronous accept: the call to async_accept returns immediately, 
         * and we use a lambda function as the handler */
        m_acceptor.async_accept(m_socket,
            [this](const boost::system::error_code& ec){
                if(!ec){
                    std::make_shared<session<Payload>>(std::move(m_socket), m_callback)->start();
                }

                start_accept();
            });
    }

    boost::asio::io_service                 m_ios;
    ba::local::stream_protocol::acceptor    m_acceptor;
    ba::local::stream_protocol::socket      m_socket;
    callback_t                              m_callback;
};

#endif /* __IPC_LISTENER_HPP__ */
