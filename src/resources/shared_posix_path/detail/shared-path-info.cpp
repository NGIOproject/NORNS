#include "resource-type.hpp"
#include "resource-info.hpp"
#include "shared-path-info.hpp"

namespace norns {
namespace data {
namespace detail {

/*! Remote path data */
shared_path_info::shared_path_info(const std::string& nsid, 
                                   const std::string& datapath)
    : m_nsid(nsid),
      m_datapath(datapath) {}

shared_path_info::~shared_path_info() { }

resource_type shared_path_info::type() const {
    return resource_type::shared_posix_path;
}

std::string shared_path_info::nsid() const {
    return m_nsid;
}

bool shared_path_info::is_remote() const {
    return false;
}

std::string shared_path_info::to_string() const {
    return "SHARED_PATH[\"" + m_nsid + "\", \"" + m_datapath + "\"]";
}

std::string shared_path_info::datapath() const {
    return m_datapath;
}

} // namespace detail
} // namespace data
} // namespace norns
