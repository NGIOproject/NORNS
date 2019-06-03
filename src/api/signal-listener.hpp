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

#ifndef __SIGNAL_LISTENER_HPP__
#define __SIGNAL_LISTENER_HPP__

#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>

namespace ba = boost::asio;

namespace norns {
namespace api {

template <class F>
void do_for(F /*f*/) {
    // Parameter pack is empty.
}

template <class F, class First, class... Rest>
void do_for(F f, First first, Rest... rest) {
    f(first);
    do_for(f, rest...);
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
