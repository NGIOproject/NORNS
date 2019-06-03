#include "resource-type.hpp"
#include "resource-info.hpp"
#include "local-path-info.hpp"

namespace norns {
namespace data {
namespace detail {

/*! Remote path data */
local_path_info::local_path_info(std::string nsid, std::string datapath)
    : m_nsid(nsid),
      m_datapath(datapath) {}

local_path_info::~local_path_info() { }

resource_type local_path_info::type() const {
    return resource_type::local_posix_path;
}

std::string local_path_info::nsid() const {
    return m_nsid;
}

bool local_path_info::is_remote() const {
    return false;
}

std::string local_path_info::to_string() const {
    return "LOCAL_PATH[\"" + m_nsid + "\", \"" + m_datapath + "\"]";
}

std::string local_path_info::datapath() const {
    return m_datapath;
}

} // namespace detail
} // namespace data
} // namespace norns
