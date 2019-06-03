//#include "resources/resource-type.hpp"
#include "resources.hpp"
#include "backend-base.hpp"
#include "lustre-fs.hpp"

namespace norns {
namespace storage {

lustre::lustre(const std::string& nsid, bool track, const bfs::path& mount, uint32_t quota) 
    : m_nsid(nsid),
      m_track(track),
      m_mount(mount), 
      m_quota(quota) { }

std::string
lustre::nsid() const {
    return m_nsid;
}

bool
lustre::is_tracked() const {
    return m_track;
}

bool
lustre::is_empty() const {
    return false;
}

bfs::path lustre::mount() const {
    return m_mount;
}

uint32_t lustre::quota() const {
    return m_quota;
}

backend::resource_ptr 
lustre::new_resource(const resource_info_ptr& rinfo, 
                     const bool is_collection, 
                     std::error_code& ec) const {
    (void) rinfo;
    (void) is_collection;
    (void) ec;
    return backend::resource_ptr();
}

backend::resource_ptr
lustre::get_resource(const resource_info_ptr& rinfo, std::error_code& ec) const {
    (void) rinfo;
    (void) ec;
    return backend::resource_ptr(); //XXX
}

void
lustre::remove(const resource_info_ptr& rinfo, std::error_code& ec) const {
    (void) rinfo;
    (void) ec;
}

std::size_t
lustre::get_size(const resource_info_ptr& rinfo, std::error_code& ec) const {
    (void) rinfo;
    (void) ec;
    return 0; //XXX
}


bool lustre::accepts(resource_info_ptr res) const {
    switch(res->type()) {
        case data::resource_type::local_posix_path:
        case data::resource_type::shared_posix_path:
            return true;
        default:
            return false;
    }
}

std::string lustre::to_string() const {
    return "LUSTRE(\"" + m_mount.string() + "\", " + std::to_string(m_quota) + ")";
}

} // namespace storage
} // namespace norns
