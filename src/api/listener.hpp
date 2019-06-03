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

#ifndef __API_LISTENER_HPP__
#define __API_LISTENER_HPP__

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "common.hpp"
#include "api/local-endpoint.hpp"
#include "api/remote-endpoint.hpp"
#include "api/signal-listener.hpp"

namespace ba = boost::asio;
namespace bfs = boost::filesystem;

namespace norns {
namespace api {

/* simple lister for an AF_UNIX socket that accepts requests asynchronously and
 * invokes a callback with a fixed-length payload */
template <typename Message>
class listener {

    using Dispatcher = dispatch_table<
        typename Message::key_type, 
        typename Message::key_hash_type, 
        std::unique_ptr<typename Message::response_type>,
        std::unique_ptr<typename Message::request_type>
            >;

    using MessageKey = typename Message::key_type;

public:
    explicit listener() :
        m_ios(),
        m_msg_handlers(std::make_shared<Dispatcher>()), 
        m_signal_listener(m_ios) { }

    ~listener() {
        Message::cleanup();
    }

    void run() {

        for(auto& endp : m_local_endpoints) {
            endp->do_accept();
        }

        for(auto& endp : m_remote_endpoints) {
            endp->do_accept();
        }

        m_signal_listener.do_accept();

        m_ios.run();
    }

    void stop() {
        m_ios.stop();
    }

    template <typename Callable>
    void register_callback(MessageKey k, Callable&& func) {
        m_msg_handlers->add(k, std::forward<Callable>(func));
    }

    /* register a socket endpoint from which to accept local requests */
    void register_endpoint(const bfs::path& sockfile) {
        m_local_endpoints.emplace_back(
                std::make_shared<local_endpoint<Message>>(sockfile, m_ios, m_msg_handlers));
    }

    /* register a socket endpoint from which to accept remote requests */
    void register_endpoint(short port) {
        m_remote_endpoints.emplace_back(
                std::make_shared<remote_endpoint<Message>>(port, m_ios, m_msg_handlers));
    }

    template <typename... Args>
    void set_signal_handler(signal_listener::SignalHandler handler, Args... signums) {
        m_signal_listener.set_handler(handler, std::forward<Args>(signums)...);
    }

    static void cleanup() {
        Message::cleanup();
    }

private:
    /*! Main io_service */
    boost::asio::io_service                m_ios;

    /*! Dispatcher of message handlers */
    std::shared_ptr<Dispatcher> m_msg_handlers;

    /*! Dispatcher of signal listener */
    signal_listener m_signal_listener;

    /*! Local endpoints */
    std::vector<std::shared_ptr<local_endpoint<Message>>>  m_local_endpoints;

    /*! Remote endpoints */
    std::vector<std::shared_ptr<remote_endpoint<Message>>> m_remote_endpoints;
};

} // namespace api
} // namespace norns

#endif /* __API_LISTENER_HPP__ */
