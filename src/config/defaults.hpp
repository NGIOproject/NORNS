#ifndef __DEFAULTS_HPP__
#define __DEFAULTS_HPP__

#include <cstdint>
#include <netinet/in.h>
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

namespace norns {
namespace config {
namespace defaults {

    extern const char*      progname;
    extern const bool       daemonize;
    extern const bool       use_syslog;
    extern const bool       use_console;
    extern const bfs::path  log_file;
    extern const uint32_t   log_file_max_size;
    extern const bool       dry_run;
    extern const uint32_t   dry_run_duration;
    extern const char*      global_socket;
    extern const char*      control_socket;
    extern const char*      bind_address;
    extern const in_port_t  remote_port;
    extern const char*      pidfile;
    extern const uint32_t   workers_in_pool;
    extern const char*      staging_directory;
    extern const uint32_t   backlog_size;
    extern const char*      config_file;

} // namespace defaults
} // namespace config
} // namespace norns

#endif /* __DEFAULTS_HPP__ */
