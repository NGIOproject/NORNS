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

#include <iostream>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/filesystem.hpp>

#include "utils.hpp"
#include "file-options.hpp"
#include "keywords.hpp"
#include "config-schema.hpp"
#include "settings.hpp"

namespace bpt = boost::property_tree;
namespace bfs = boost::filesystem;

namespace norns {
namespace config {

settings::settings() { 
    this->load_defaults();
}

settings::settings(const std::string& progname, 
                   bool daemonize, 
                   bool use_syslog,
                   bool use_console,
                   const bfs::path& log_file, 
                   const uint32_t log_file_max_size,
                   bool dry_run, 
                   uint32_t dry_run_duration, 
                   const bfs::path& global_socket, 
                   const bfs::path& control_socket, 
                   const std::string& configured_address,
                   uint32_t remote_port,
                   const bfs::path& pidfile, 
                   uint32_t workers,
                   const bfs::path& staging_directory,
                   uint32_t backlog_size, 
                   const bfs::path& cfgfile, 
                   const std::list<namespace_def>& defns) :
    m_progname(progname),
    m_daemonize(daemonize),
    m_use_syslog(use_syslog),
    m_use_console(use_console),
    m_log_file(log_file),
    m_log_file_max_size(log_file_max_size),
    m_dry_run(dry_run),
    m_dry_run_duration(dry_run_duration),
    m_global_socket(global_socket),
    m_control_socket(control_socket),
    m_configured_address(configured_address),
    m_lookup_address(),
    m_remote_port(remote_port),
    m_daemon_pidfile(pidfile),
    m_workers_in_pool(workers),
    m_staging_directory(staging_directory),
    m_backlog_size(backlog_size),
    m_config_file(cfgfile),
    m_default_namespaces(defns) { }

void 
settings::load_defaults() {
    m_progname = defaults::progname;
    m_daemonize = defaults::daemonize;
    m_use_syslog = defaults::use_syslog;
    m_use_console = defaults::use_console;
    m_log_file = defaults::log_file;
    m_log_file_max_size = defaults::log_file_max_size;
    m_dry_run = defaults::dry_run;
    m_dry_run_duration = defaults::dry_run_duration;
    m_global_socket = defaults::global_socket;
    m_control_socket = defaults::control_socket;
    m_configured_address = defaults::bind_address;
    m_lookup_address.clear();
    m_remote_port = defaults::remote_port;
    m_daemon_pidfile = defaults::pidfile;
    m_workers_in_pool = defaults::workers_in_pool;
    m_staging_directory = defaults::staging_directory;
    m_backlog_size = defaults::backlog_size;
    m_config_file = defaults::config_file;
    m_default_namespaces.clear();
}

void 
settings::load_from_file(const bfs::path& filename) {

    file_options::options_map opt_map;
    file_options::parse_yaml_file(filename, config::valid_options, opt_map);

    // load global settings
    const auto& gsettings = 
        opt_map.get_as<file_options::options_group>(keywords::global_settings);

    m_progname = defaults::progname;
    m_use_syslog = gsettings.get_as<bool>(keywords::use_syslog);
    m_use_console = defaults::use_console;

    if(gsettings.has(keywords::log_file)) {
        m_log_file = gsettings.get_as<bfs::path>(keywords::log_file);
    }

    if(gsettings.has(keywords::log_file_max_size)) {
        m_log_file_max_size = 
            gsettings.get_as<uint32_t>(keywords::log_file_max_size);
    }

    m_dry_run = gsettings.get_as<bool>(keywords::dry_run);
    m_dry_run_duration = defaults::dry_run_duration;
    m_global_socket = gsettings.get_as<bfs::path>(keywords::global_socket);
    m_control_socket = gsettings.get_as<bfs::path>(keywords::control_socket);
    m_configured_address = 
        gsettings.get_as<std::string>(keywords::bind_address);
    m_lookup_address.clear();
    m_remote_port = gsettings.get_as<uint32_t>(keywords::remote_port);
    m_daemon_pidfile = gsettings.get_as<bfs::path>(keywords::pidfile);
    m_workers_in_pool = gsettings.get_as<uint32_t>(keywords::workers);
    m_staging_directory =
        gsettings.get_as<bfs::path>(keywords::staging_directory);
    m_backlog_size = defaults::backlog_size;

    // load definitions for default namespaces
    const auto& namespaces =
        opt_map.get_as<file_options::options_list>(keywords::namespaces);

    for(const auto& nsdef : namespaces) {
        m_default_namespaces.emplace_back(
                nsdef.get_as<std::string>(keywords::nsid),
                nsdef.get_as<bool>(keywords::track_contents),
                nsdef.get_as<bfs::path>(keywords::mountpoint),
                nsdef.get_as<std::string>(keywords::type),
                nsdef.get_as<uint64_t>(keywords::capacity),
                nsdef.get_as<std::string>(keywords::visibility));
    }
}

std::string 
settings::to_string() const {
    std::string str = 
        std::string("settings {\n") +
           "  m_progname: " + m_progname + ",\n" +
           "  m_daemonize: " + (m_daemonize ? "true" : "false") + ",\n" +
           "  m_use_syslog: " + (m_use_syslog ? "true" : "false") +  ",\n" +
           "  m_use_console: " + (m_use_console ? "true" : "false") +  ",\n" +
           "  m_log_file: " + m_log_file.string() + ",\n" +
           "  m_log_file_max_size: " + 
                std::to_string(m_log_file_max_size) + ",\n" +
           "  m_dry_run: " + (m_dry_run ? "true" : "false") +  ",\n" +
           "  m_dry_run_duration: " + 
                std::to_string(m_dry_run_duration) +  ",\n" +
           "  m_global_socket: " + m_global_socket.string() + ",\n" +
           "  m_control_socket: " + m_control_socket.string() + ",\n" +
           "  m_configured_address: " + m_configured_address + ",\n" +
           "  m_lookup_address: " + 
                (m_lookup_address.empty() ?  
                 m_lookup_address : "{undetermined}") + ",\n" +
           "  m_remote_port: " + std::to_string(m_remote_port) + ",\n" +
           "  m_pidfile: " + m_daemon_pidfile.string() + ",\n" +
           "  m_workers: " + std::to_string(m_workers_in_pool) + ",\n" +
           "  m_staging_directory: " + m_staging_directory.string() + ",\n" +
           "  m_backlog_size: " + std::to_string(m_backlog_size) + ",\n" +
           "  m_config_file: " + m_config_file.string() + ",\n" +
           "};";
    //TODO: add m_default_namespaces
    return str;
}

std::string
settings::progname() const {
    return m_progname;
}

void
settings::progname(const std::string& progname) {
    m_progname = progname;
}

bool
settings::daemonize() const {
    return m_daemonize;
}

void
settings::daemonize(bool daemonize) {
    m_daemonize = daemonize;
}

bool
settings::use_syslog() const {
    return m_use_syslog;
}

void
settings::use_syslog(bool use_syslog) {
    m_use_syslog = use_syslog;
}

bool
settings::use_console() const {
    return m_use_console;
}

void
settings::use_console(bool use_console) {
    m_use_console = use_console;
}

bfs::path
settings::log_file() const {
    return m_log_file;
}

void
settings::log_file(const bfs::path& log_file) {
    m_log_file = log_file;
}

uint32_t
settings::log_file_max_size() const {
    return m_log_file_max_size;
}

void
settings::log_file_max_size(uint32_t log_file_max_size) {
    m_log_file_max_size = log_file_max_size;
}

bool
settings::dry_run() const {
    return m_dry_run;
}

void
settings::dry_run(bool dry_run) {
    m_dry_run = dry_run;
}

uint32_t
settings::dry_run_duration() const {
    return m_dry_run_duration;
}

void
settings::dry_run_duration(uint32_t dry_run_duration) {
    m_dry_run_duration = dry_run_duration;
}

bfs::path
settings::global_socket() const {
    return m_global_socket;
}

void
settings::global_socket(const bfs::path& global_socket) {
    m_global_socket = global_socket;
}

bfs::path
settings::control_socket() const {
    return m_control_socket;
}

void
settings::control_socket(const bfs::path& control_socket) {
    m_control_socket = control_socket;
}

std::string
settings::configured_address() const {
    return m_configured_address;
}

void
settings::configured_address(const std::string& configured_address) {
    m_configured_address = configured_address;
}

std::string
settings::lookup_address() const {
    return m_lookup_address;
}

void
settings::lookup_address(const std::string& lookup_address) {
    m_lookup_address = lookup_address;
}

in_port_t
settings::remote_port() const {
    return m_remote_port;
}

void
settings::remote_port(in_port_t remote_port) {
    m_remote_port = remote_port;
}

bfs::path
settings::pidfile() const {
    return m_daemon_pidfile;
}

void
settings::pidfile(const bfs::path& pidfile) {
    m_daemon_pidfile = pidfile;
}

uint32_t
settings::workers_in_pool() const {
    return m_workers_in_pool;
}

void
settings::workers_in_pool(uint32_t workers_in_pool) {
    m_workers_in_pool = workers_in_pool;
}

bfs::path
settings::staging_directory() const {
    return m_staging_directory;
}

void
settings::staging_directory(const bfs::path& staging_directory) {
    m_staging_directory = staging_directory;
}

uint32_t
settings::backlog_size() const {
    return m_backlog_size;
}

void
settings::backlog_size(uint32_t backlog_size) {
    m_backlog_size = backlog_size;
}

bfs::path 
settings::config_file() const {
    return m_config_file;
}

void
settings::config_file(const bfs::path& config_file) {
    m_config_file = config_file;
}

std::list<namespace_def>
settings::default_namespaces() const {
    return m_default_namespaces;
}

void 
settings::default_namespaces(
    const std::list<namespace_def>& default_namespaces) {
    m_default_namespaces = default_namespaces;
}

} // namespace config
} // namespace norns
