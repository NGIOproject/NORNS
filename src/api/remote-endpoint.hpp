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

#ifndef __REMOTE_ENDPOINT_HPP__
#define __REMOTE_ENDPOINT_HPP__

#include <memory>
#include <boost/asio.hpp>

#include "api/session.hpp"

namespace ba = boost::asio;
namespace bfs = boost::filesystem;

namespace norns {
namespace api {

template <typename Message>
class remote_endpoint {

    using SessionType = session<ba::ip::tcp::socket, Message>;
    using DispatcherType = typename SessionType::Dispatcher;

public: 
    remote_endpoint(short port, ba::io_service& ios,
                    std::shared_ptr<DispatcherType> dispatcher) :
        m_port(port),
        m_socket(ios),
        m_acceptor(ios, ba::ip::tcp::endpoint(ba::ip::tcp::v4(), port)),
        m_dispatcher(dispatcher) {

        m_acceptor.set_option(ba::ip::tcp::acceptor::reuse_address(true));
    }

    ~remote_endpoint() { }

    void do_accept() {
        m_acceptor.async_accept(m_socket, 
            [this](const boost::system::error_code& ec) {
                if(!ec) {
                    std::make_shared<SessionType>(
                            std::move(m_socket),
                            m_dispatcher
                        )->start();
                }

                do_accept();
            }
        );
    }

private:
    short m_port;
    ba::ip::tcp::socket m_socket;
    ba::ip::tcp::acceptor m_acceptor;
    std::shared_ptr<DispatcherType> m_dispatcher;
};

} // namespace api
} // namespace norns

#endif /* __REMOTE_ENDPOINT_HPP__ */
