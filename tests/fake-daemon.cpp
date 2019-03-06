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


#ifdef DEBUG_OUTPUT
#include <iostream>
#endif

#include <chrono>
#include "nornsctl.h"
#include "fake-daemon.hpp"

const norns::config::settings fake_daemon::default_cfg(
    "test_urd", /* progname */
    false, /* daemonize */
    false, /* use syslog */
    false, /* use console */
    {},// "./test_urd.log", /* log file */
    0, /* unused */
    false, /* dry run */
    100, /* dry run duration */
    "./test_urd.global.socket", /* global_socket */
    "./test_urd.control.socket", /* control_socket */
    "127.0.0.1", /* bind address */
    42002, /* remote port */
    "./test_urd.pid", /* daemon_pidfile */
    2, /* api workers */
    "./tmp/", /* staging directory */
    128,
    "./",
    {}
);

fake_daemon::fake_daemon() {
//    extern const char* norns_api_global_socket;
//    extern const char* norns_api_control_socket;
//    norns_api_global_socket = "./test_urd.global.socket";
//    norns_api_control_socket = "./test_urd.control.socket";
}

fake_daemon::~fake_daemon() {
    if(m_running) {
        stop();
    }
}

void fake_daemon::configure(const bfs::path& config_file, 
                            const fake_daemon_cfg& override_cfg) {

#if 1
    // most settings are taken from the default configuration for tests
    m_config = default_cfg;

    // some settings are taken from the configuration file auto-generated 
    // for this test (mostly those path-related)
    norns::config::settings file_config;
    file_config.load_from_file(config_file);

    m_config.log_file(file_config.log_file());
    m_config.global_socket(file_config.global_socket());
    m_config.control_socket(file_config.control_socket());
    m_config.pidfile(file_config.pidfile());
    m_config.staging_directory(file_config.staging_directory());
    m_config.config_file(config_file);

    // we allow some settings to be overriden
    m_config.dry_run(override_cfg.m_dry_run);
    m_config.dry_run_duration(override_cfg.m_dry_run_duration);

    // for now default_namespaces is empty
    m_config.default_namespaces().clear();

#endif

#if 0
    m_config.load_from_file(config_file);

    m_config.progname(default_cfg.progname());
    m_config.daemonize(default_cfg.daemonize());
    m_config.use_syslog(default_cfg.use_syslog());
    m_config.use_console(default_cfg.use_console());
    m_config.dry_run(override_cfg.m_dry_run);
    m_config.dry_run_duration(override_cfg.m_dry_run_duration);
    m_config.bind_address(default_cfg.bind_address());
    m_config.remote_port(default_cfg.remote_port());
    m_config.workers_in_pool(default_cfg.workers_in_pool());
    m_config.staging_directory(default_cfg.staging_directory());
    m_config.config_file(config_file);
    m_config.default_namespaces().clear();
#endif
}

void fake_daemon::configure(const bfs::path& config_file,
                            const std::string& alias) {

#if 1
    // most settings are taken from the configuration file auto-generated 
    // for this test (mostly those path-related)
    m_config.load_from_file(config_file);

    m_config.progname(default_cfg.progname() + alias);
    m_config.daemonize(default_cfg.daemonize());
    m_config.use_console(default_cfg.use_console());
    m_config.config_file(config_file);
#endif

#if 0
    m_config.load_from_file(config_file);

    m_config.progname(default_cfg.progname() + alias);
    m_config.daemonize(default_cfg.daemonize());
    m_config.use_syslog(default_cfg.use_syslog());
    m_config.use_console(default_cfg.use_console());
    m_config.dry_run(default_cfg.dry_run());
    m_config.dry_run_duration(default_cfg.dry_run_duration());
    m_config.bind_address(default_cfg.bind_address());
    m_config.remote_port(default_cfg.remote_port());
    m_config.workers_in_pool(default_cfg.workers_in_pool());
    m_config.staging_directory(default_cfg.staging_directory());
    m_config.config_file(config_file);
    m_config.default_namespaces().clear();
#endif
}

extern "C" void __gcov_flush (void);

void fake_daemon::run() {

    m_running = true;
    m_pid = fork();

    switch(m_pid) {
        /* child code */
        case 0:
        {
            // disable any signal handlers set by Catch2
            // so that we can receive control signals from the parent process
            for(auto signum : { SIGINT, SIGILL, SIGFPE, 
                                SIGSEGV, SIGTERM, SIGABRT }) {

                struct sigaction sa;
                sa.sa_handler = SIG_DFL;

                if(::sigaction(signum, &sa, NULL) != 0) {
                    throw std::runtime_error("Failed to reset Catch2 signal handler");
                }
            }

            m_daemon.configure(m_config);
            m_daemon.run();
            m_daemon.teardown();

#ifdef DEBUG_OUTPUT
            std::cerr << "[" << getpid() << "] exitting...\n";
#endif

            __gcov_flush();
            exit(0);
        }

        /* error code */
        case -1:
            throw std::runtime_error("Failed to spawn test daemon");

        /* parent code */
        default:
        {

#ifdef DEBUG_OUTPUT
            std::cerr << "[" << getpid() << "] daemon process spawned (" << m_pid << ")\n";
#endif

            int rv;
            int retries = 20;

            do {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                rv = nornsctl_send_command(NORNSCTL_CMD_PING, NULL);
            } while(rv != NORNS_SUCCESS && --retries != 0);

            if(retries == 0) {
                // the daemon may be running even if we don't receive a reply,
                // try to stop it to avoid leaving a dangling process
                stop();
                throw std::runtime_error("Failed to ping test daemon");
            }

#ifdef DEBUG_OUTPUT
            std::cerr << "[" << getpid() << "] daemon process ready\n";
#endif
        }
    }
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
