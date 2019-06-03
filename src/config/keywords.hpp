#ifndef __CONFIG_KEYWORDS_HPP__
#define __CONFIG_KEYWORDS_HPP__

namespace norns {
namespace config {

// define some constant keywords so that we can refer to them throughout
// the parsing code without having to rely on the actual string literal
// (this will allow us to rename options in the configuration file
// without having to modify all the statements that refer to them)
namespace keywords {

// section names
constexpr static const auto global_settings = "global_settings";
constexpr static const auto namespaces = "namespaces";

// option names for 'global-settings' section
constexpr static const auto use_syslog = "use_syslog";
constexpr static const auto log_file = "log_file";
constexpr static const auto log_file_max_size = "log_file_max_size";
constexpr static const auto dry_run = "dry_run";
constexpr static const auto global_socket = "global_socket";
constexpr static const auto control_socket = "control_socket";
constexpr static const auto bind_address = "bind_address";
constexpr static const auto remote_port = "remote_port";
constexpr static const auto pidfile = "pidfile";
constexpr static const auto workers = "workers";
constexpr static const auto staging_directory = "staging_directory";

// option names for 'namespaces' section
constexpr static const auto nsid = "nsid";
constexpr static const auto track_contents = "track_contents";
constexpr static const auto mountpoint = "mountpoint";
constexpr static const auto type = "type";
constexpr static const auto capacity = "capacity";
constexpr static const auto visibility = "visibility";

}

} // namespace config
} // namespace norns

#endif /* __CONFIG_KEYWORDS_HPP__ */

