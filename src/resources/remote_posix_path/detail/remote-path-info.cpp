#include "resource-type.hpp"
#include "resource-info.hpp"
#include "remote-path-info.hpp"

namespace norns {
namespace data {
namespace detail {

/*! Remote path data */
remote_path_info::remote_path_info(const std::string& nsid, 
                                   const std::string& hostname, 
                                   const std::string& datapath)
    : m_nsid(nsid),
      m_hostname(hostname),
      m_datapath(datapath) {}

remote_path_info::~remote_path_info() { }

resource_type remote_path_info::type() const {
    return resource_type::remote_posix_path;
}

std::string remote_path_info::nsid() const {
    return m_nsid;
}

bool remote_path_info::is_remote() const {
    return true;
}

std::string remote_path_info::to_string() const {
    return "REMOTE_PATH[\"" + m_hostname + "\", \"" + m_nsid + "\", \"" + m_datapath + "\"]";
}

std::string 
remote_path_info::hostname() const {
    return m_hostname;
}

std::string remote_path_info::datapath() const {
    return m_datapath;
}

} // namespace detail
} // namespace data
} // namespace norns
