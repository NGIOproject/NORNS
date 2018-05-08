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

#ifndef __LOCAL_ENDPOINT_HPP__
#define __LOCAL_ENDPOINT_HPP__

#include <memory>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "api/session.hpp"

namespace ba = boost::asio;
namespace bfs = boost::filesystem;

namespace norns {
namespace api {

template <typename Message>
class local_endpoint {

    using DispatcherType = typename session<Message>::Dispatcher;

public:
    local_endpoint(const bfs::path& sockfile, ba::io_service& ios, 
                   std::shared_ptr<DispatcherType> dispatcher) :
        m_sockfile(sockfile),
        m_socket(ios),
        m_acceptor(ios, ba::local::stream_protocol::endpoint(sockfile.string())),
        m_dispatcher(dispatcher) { }

    ~local_endpoint() {
        bfs::remove(m_sockfile);
    }

    void do_accept() {
        m_acceptor.async_accept(m_socket, 
            [this](const boost::system::error_code& ec) {
                if(!ec) {
                    std::make_shared<session<Message>>(
                            std::move(m_socket), m_dispatcher
                        )->start();
                }

                do_accept();
            }
        );
    }

private:
    bfs::path m_sockfile;
    ba::local::stream_protocol::socket m_socket;
    ba::local::stream_protocol::acceptor m_acceptor;
    std::shared_ptr<DispatcherType> m_dispatcher;
};

} // namespace api
} // namespace norns

#endif /* __LOCAL_ENDPOINT_HPP__ */
