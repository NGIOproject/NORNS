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

#ifndef __FAKE_DAEMON_HPP__
#define  __FAKE_DAEMON_HPP__

#include <sys/types.h>
#include <sys/wait.h>

#include <urd.hpp>
#include <config.hpp>

struct fake_daemon_cfg {

    fake_daemon_cfg(bool dry_run, uint32_t t = 100) :
        m_dry_run(dry_run),
        m_dry_run_duration(t) { }

    bool m_dry_run = false;
    uint32_t m_dry_run_duration = 0;
};

struct fake_daemon {

    static const norns::config::settings default_cfg;

    fake_daemon();
    ~fake_daemon();
    void configure(const bfs::path& config_file, const fake_daemon_cfg& override_cfg);
    void configure(const bfs::path& config_file, const std::string& alias = "");
    void run();
    int stop();

    pid_t m_pid = 0;
    bool m_running = false;
    norns::urd m_daemon;
    norns::config::settings m_config;
};

#endif /* __FAKE_DAEMON_HPP__ */

