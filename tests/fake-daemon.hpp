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

#ifndef __FAKE_DAEMON_HPP__
#define  __FAKE_DAEMON_HPP__

#include <sys/types.h>
#include <sys/wait.h>

#include <urd.hpp>
#include <config.hpp>

struct fake_daemon_cfg {

    fake_daemon_cfg(bool dry_run) :
        m_dry_run(dry_run) { }

    bool m_dry_run = false;
};

struct fake_daemon {

    fake_daemon();
    ~fake_daemon();
    void configure(const bfs::path& config_file, const fake_daemon_cfg& override_cfg);
    void configure(const bfs::path& config_file);
    void run();
    int stop();

    pid_t m_pid = 0;
    bool m_running = false;
    norns::urd m_daemon;
    norns::config::settings m_config;
};

#endif /* __FAKE_DAEMON_HPP__ */

