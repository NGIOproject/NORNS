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

#ifndef __SIGNAL_LISTENER_HPP__
#define __SIGNAL_LISTENER_HPP__

#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>

namespace ba = boost::asio;

namespace norns {
namespace api {

template <class F, class First, class... Rest>
void do_for(F f, First first, Rest... rest) {
    f(first);
    do_for(f, rest...);
}

template <class F>
void do_for(F /*f*/) {
    // Parameter pack is empty.
}

struct signal_listener {

    using SignalHandler = std::function<void(int)>;

    explicit signal_listener(ba::io_service& ios) :
        m_signals(ios) {}

    template <typename... Args>
    void set_handler(const SignalHandler& handler, Args... signums) {

        m_handler = std::move(handler);

        m_signals.clear();

        do_for(
                [&](int signum) {
                    m_signals.add(signum);
                }, 
                signums...);
    }

    void clear_handler() {
        m_handler = nullptr;
        m_signals.clear();
    }

    void do_accept() {

        if(m_handler) {
            m_signals.async_wait(
                std::bind(&signal_listener::signal_handler, this,
                    std::placeholders::_1, std::placeholders::_2));
        }
    }

private:
    void signal_handler(boost::system::error_code error, int signal_number) {
        if(!error) {

            // a signal occurred, invoke handler
            if(m_handler) {
                m_handler(signal_number);
            }

            // reinstall handler
            m_signals.async_wait(
                std::bind(&signal_listener::signal_handler, this, 
                    std::placeholders::_1, std::placeholders::_2));
        }
    }

    ba::io_service m_ios;
    ba::signal_set m_signals;
    SignalHandler  m_handler;
};

} // namespace api
} // namespace norns

#endif /* __SIGNAL_LISTENER_HPP__ */
