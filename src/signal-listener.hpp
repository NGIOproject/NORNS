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

#ifndef __SIGNAL_LISTENER_HPP__
#define __SIGNAL_LISTENER_HPP__

#include <thread>
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>

namespace ba = boost::asio;

class signal_listener {

    typedef std::function<void(int)> callback_t;

public:
    signal_listener(callback_t callback) 
    : m_signals(m_ios, SIGHUP, SIGTERM),
      m_callback(callback) {
        m_signals.async_wait(
            std::bind(&signal_listener::handler, this, std::ref(m_signals), 
                std::placeholders::_1, std::placeholders::_2));
    }

    void run() {
        std::thread(
            [&](){
                m_ios.run();
            }).detach();
    }

private:
    void handler(boost::asio::signal_set& signal_set, boost::system::error_code error, int signal_number) {
        if(!error) {
            // a signal occurred, invoke callback
            m_callback(signal_number);

            // reinstall 
            m_signals.async_wait(
                std::bind(&signal_listener::handler, this, std::ref(signal_set), 
                    std::placeholders::_1, std::placeholders::_2));
        }
    }

private:
    ba::io_service m_ios;
    ba::signal_set m_signals;
    callback_t     m_callback;
};

#endif /* __SIGNAL_LISTENER_HPP__ */
