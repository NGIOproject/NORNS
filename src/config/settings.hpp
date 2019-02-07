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

#ifndef __SETTINGS_HPP__
#define __SETTINGS_HPP__

#include <list>
#include <boost/filesystem.hpp>
#include <netinet/in.h>

namespace bfs = boost::filesystem;

namespace norns {
namespace config {

struct namespace_def {

    namespace_def(const std::string& nsid,
                  bool track,
                  const bfs::path& mountpoint,
                  const std::string& alias,
                  const uint64_t capacity,
                  const std::string& visibility) :
        m_nsid(nsid),
        m_track(track),
        m_mountpoint(mountpoint),
        m_alias(alias),
        m_capacity(capacity),
        m_visibility(visibility) { }

    std::string nsid() const {
        return m_nsid;
    }

    bool track() const {
        return m_track;
    }

    bfs::path mountpoint() const {
        return m_mountpoint;
    }

    std::string alias() const {
        return m_alias;
    }

    uint64_t capacity() const {
        return m_capacity;
    }

    std::string visibility() const {
        return m_visibility;
    }

    const std::string m_nsid;
    const bool        m_track;
    const bfs::path   m_mountpoint;
    const std::string m_alias;
    const uint64_t    m_capacity;
    const std::string m_visibility;
};

struct settings {

    settings();
    settings(const std::string& progname,
             bool daemonize,
             bool use_syslog,
             bool use_console,
             const bfs::path& log_file,
             const uint32_t log_file_max_size,
             bool dry_run,
             uint32_t dry_run_duration,
             const bfs::path& global_socket,
             const bfs::path& control_socket,
             const std::string& bind_address,
             uint32_t remote_port,
             const bfs::path& pidfile,
             uint32_t workers,
             uint32_t backlog_size,
             const bfs::path& cfgfile,
             const std::list<namespace_def>& defns);
    void load_defaults();
    void load_from_file(const bfs::path& filename);
    std::string to_string() const;

    std::string& progname();
    bool& daemonize();
    bool& use_syslog();
    bool& use_console();
    bfs::path& log_file();
    uint32_t& log_file_max_size();
    bool& dry_run();
    uint32_t& dry_run_duration();
    bfs::path& global_socket();
    bfs::path& control_socket();
    std::string& bind_address();
    in_port_t& remote_port();
    bfs::path& pidfile();
    uint32_t& workers_in_pool();
    uint32_t& backlog_size();
    bfs::path& config_file();
    std::list<namespace_def>& default_namespaces();

    std::string m_progname = defaults::progname;
    bool        m_daemonize = defaults::daemonize;
    bool        m_use_syslog = defaults::use_syslog;
    bool        m_use_console = defaults::use_console;
    bfs::path   m_log_file = defaults::log_file;
    uint32_t    m_log_file_max_size = defaults::log_file_max_size;
    bool        m_dry_run = defaults::dry_run;
    uint32_t    m_dry_run_duration = defaults::dry_run_duration;
    bfs::path   m_global_socket = defaults::global_socket;
    bfs::path   m_control_socket = defaults::control_socket;
    std::string m_bind_address = defaults::bind_address;
    in_port_t   m_remote_port = defaults::remote_port;
    bfs::path   m_daemon_pidfile = defaults::pidfile;
    uint32_t    m_workers_in_pool = defaults::workers_in_pool;
    uint32_t    m_backlog_size = defaults::backlog_size;
    bfs::path   m_config_file = defaults::config_file;
    std::list<namespace_def> m_default_namespaces;
};

} // namespace config
} // namespace norns

#endif /* __SETTINGS_HPP__ */
