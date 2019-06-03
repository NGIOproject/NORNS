#include "backend-base.hpp"
#include "resources.hpp"
#include "remote-backend.hpp"
#include "logger.hpp"

namespace norns {
namespace storage {
namespace detail {

remote_backend::remote_backend(const std::string& nsid) :
    m_nsid(nsid) {} 

std::string
remote_backend::nsid() const {
    return m_nsid;
}

bool
remote_backend::is_tracked() const {
    return false;
}

bool
remote_backend::is_empty() const {
    return false;
}

bfs::path 
remote_backend::mount() const {
    return "";
}

uint32_t 
remote_backend::quota() const {
    return 0;
}

backend::resource_ptr 
remote_backend::new_resource(const resource_info_ptr& rinfo, 
                             const bool is_collection,
                             std::error_code& ec) const {
    (void) is_collection;
    (void) ec;

    const auto d_rinfo = 
        std::static_pointer_cast<data::remote_resource_info>(rinfo);

    return std::make_shared<data::remote_resource>(shared_from_this(), d_rinfo);
}

backend::resource_ptr
remote_backend::get_resource(const resource_info_ptr& rinfo, 
                             std::error_code& ec) const {
    (void) ec;

    const auto d_rinfo = 
        std::static_pointer_cast<data::remote_resource_info>(rinfo);

    return std::make_shared<data::remote_resource>(shared_from_this(), d_rinfo);
}

void
remote_backend::remove(const resource_info_ptr& rinfo, 
                       std::error_code& ec) const {
    (void) rinfo;
    (void) ec;
}

std::size_t
remote_backend::get_size(const resource_info_ptr& rinfo, 
                         std::error_code& ec) const {
    (void) rinfo;
    (void) ec;

    return 0; //XXX
}

bool 
remote_backend::accepts(resource_info_ptr res) const {
    switch(res->type()) {
        case data::resource_type::local_posix_path:
            return true;
        default:
            return false;
    }
}

std::string remote_backend::to_string() const {
    return "REMOTE_BACKEND";// + m_mount + ", " + std::to_string(m_quota) + ")";
}

} // namespace detail
} // namespace storage
} // namespace norns
