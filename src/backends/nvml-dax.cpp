#include "backend-base.hpp"
#include "resources.hpp"
#include "utils.hpp"
#include "nvml-dax.hpp"

namespace norns {
namespace storage {

nvml_dax::nvml_dax(const std::string& nsid, bool track, const bfs::path& mount, uint32_t quota) 
    : m_nsid(nsid),
      m_track(track),
      m_mount(mount),
      m_quota(quota) { }

std::string 
nvml_dax::nsid() const {
    return m_nsid;
}

bool
nvml_dax::is_tracked() const {
    return m_track;
}

bool
nvml_dax::is_empty() const {
    return false;
}

bfs::path nvml_dax::mount() const {
    return m_mount;
}

uint32_t nvml_dax::quota() const {
    return m_quota;
}

backend::resource_ptr 
nvml_dax::new_resource(const resource_info_ptr& rinfo, 
                       const bool is_collection, 
                       std::error_code& ec) const {
    (void) rinfo;
    (void) is_collection;
    (void) ec;
    return std::make_shared<data::local_path_resource>(shared_from_this(), ""); //XXX
}

backend::resource_ptr
nvml_dax::get_resource(const resource_info_ptr& rinfo, std::error_code& ec) const {
    (void) rinfo;
    (void) ec;
    return std::make_shared<data::local_path_resource>(shared_from_this(), ""); //XXX
}

void
nvml_dax::remove(const resource_info_ptr& rinfo, std::error_code& ec) const {
    (void) rinfo;
    (void) ec;
}

std::size_t
nvml_dax::get_size(const resource_info_ptr& rinfo, std::error_code& ec) const {
    (void) rinfo;
    (void) ec;

    return 0; //XXX
}

bool nvml_dax::accepts(resource_info_ptr res) const {
    switch(res->type()) {
        case data::resource_type::local_posix_path:
            return true;
        default:
            return false;
    }
}

std::string nvml_dax::to_string() const {
    return "NORNS_NVML(\"" + m_mount.string() + "\", " + std::to_string(m_quota) + ")";
}

} // namespace storage
} // namespace norns
