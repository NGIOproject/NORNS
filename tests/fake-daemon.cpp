/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 *                                                                       *
 * This file is part of the Data Scheduler, a daemon for tracking and    *
 * managing requests for asynchronous data transfer in a hierarchical    *
 * storage environment.                                                  *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The Data Scheduler is free software: you can redistribute it and/or   *
 * modify it under the terms of the GNU Lesser General Public License    *
 * as published by the Free Software Foundation, either version 3 of     *
 * the License, or (at your option) any later version.                   *
 *                                                                       *
 * The Data Scheduler is distributed in the hope that it will be useful, *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with Data Scheduler.  If not, see                *
 * <http://www.gnu.org/licenses/>.                                       *
 *                                                                       *
 *************************************************************************/


#ifdef DEBUG_OUTPUT
#include <iostream>
#endif

#include <chrono>
#include "fake-daemon.hpp"

fake_daemon::fake_daemon() {
    extern const char* norns_api_sockfile;
    norns_api_sockfile = "./test_urd.socket";
}

fake_daemon::~fake_daemon() {
    if(m_running) {
        stop();
    }
}

void fake_daemon::run() {

    m_running = true;

    m_pid = fork();

    if(m_pid != 0) {

#ifdef DEBUG_OUTPUT
        std::cerr << "[" << getpid() << "] daemon process spawned (" << m_pid << ")\n";
#endif

        int rv;

        do {
            std::this_thread::sleep_for(std::chrono::microseconds(200));
            rv = norns_ping();
        } while(rv != NORNS_SUCCESS);

#ifdef DEBUG_OUTPUT
        std::cerr << "[" << getpid() << "] daemon process ready\n";
#endif

        return;
    }

    config_settings settings = {
        "test_urd", /* progname */
        false, /* daemonize */
        true, /* use syslog */
        "./", /* running_dir */
        "./test_urd.socket", /* api_sockfile */
        "./test_urd.pid", /* daemon_pidfile */
        2, /* api workers */
        "./",
        42,
        {}
    };

    m_daemon.configure(settings);
    m_daemon.run();
    m_daemon.teardown();

#ifdef DEBUG_OUTPUT
    std::cerr << "[" << getpid() << "] exitting...\n";
#endif

    exit(0);
}

int fake_daemon::stop() {

    if(!m_running) {
        return 0;
    }

    if(m_pid != 0) {

#ifdef DEBUG_OUTPUT
        std::cerr << "[" << getpid() << "] Sending SIGTERM to " << m_pid << "\n";
#endif

        if(kill(m_pid, SIGTERM) != 0) {

#ifdef DEBUG_OUTPUT
            std::cerr << "[" << getpid() << "] Unable to send SIGTERM to daemon process: " << strerror(errno) << "\n";
#endif

            return -1;
        }

        int status;

        if(waitpid(m_pid, &status, 0) == -1) {

#ifdef DEBUG_OUTPUT
            std::cerr << "[" << getpid() << "] Unable to wait for daemon process\n";
#endif

            return -1;
        }

#ifdef DEBUG_OUTPUT
        std::cerr << "[" << getpid() << "] Daemon process exited with status " << status << "\n";
#endif

        m_running = false;
    }

    return 0;
}
