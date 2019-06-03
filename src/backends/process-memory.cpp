#include "backend-base.hpp"
#include "resources.hpp"
#include "process-memory.hpp"

namespace norns {
namespace storage {
namespace detail {

process_memory::process_memory(const std::string& nsid) :
    m_nsid(nsid) { }

std::string
process_memory::nsid() const {
    return m_nsid;
}

bool 
process_memory::is_tracked() const {
    return false;
}

bool
process_memory::is_empty() const {
    return false;
}

bfs::path process_memory::mount() const {
    return "";
}

uint32_t process_memory::quota() const {
    return 0;
}

backend::resource_ptr 
process_memory::new_resource(const resource_info_ptr& rinfo, 
                             const bool is_collection, 
                             std::error_code& ec) const {
    (void) rinfo;
    (void) is_collection;
    (void) ec;
    return nullptr;
}

backend::resource_ptr 
process_memory::get_resource(const resource_info_ptr& rinfo, 
                             std::error_code& ec) const {

    const auto d_rinfo = std::static_pointer_cast<data::memory_region_info>(rinfo);

    ec = std::error_code();
    return std::make_shared<data::memory_region_resource>(
            shared_from_this(), d_rinfo->address(), d_rinfo->size());
}

void
process_memory::remove(const resource_info_ptr& rinfo, std::error_code& ec) const {
    (void) rinfo;
    (void) ec;
}

std::size_t
process_memory::get_size(const resource_info_ptr& rinfo, std::error_code& ec) const {

    const auto d_rinfo = std::static_pointer_cast<data::memory_region_info>(rinfo);

    ec = std::error_code();
    return d_rinfo->size();
}

bool process_memory::accepts(resource_info_ptr res) const {
    switch(res->type()) {
        case data::resource_type::memory_region:
            return true;
        default:
            return false;
    }
}

std::string process_memory::to_string() const {
    return "PROCESS_MEMORY";
}

} // namespace detail
} // namespace storage
} // namespace norns

