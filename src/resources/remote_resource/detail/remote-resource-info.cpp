#include "resource-type.hpp"
#include "resource-info.hpp"
#include "remote-resource-info.hpp"

namespace norns {
namespace data {
namespace detail {

/*! Remote path data */
remote_resource_info::remote_resource_info(
        const std::string& address,
        const std::string& nsid,
        const std::string& name) :
    m_address(address),
    m_nsid(nsid),
    m_is_collection(false),
    m_name(name) { }

remote_resource_info::remote_resource_info(
        const std::string& address,
        const std::string& nsid,
        bool is_collection,
        const std::string& name,
        const hermes::exposed_memory& buffers) :
    m_address(address),
    m_nsid(nsid),
    m_is_collection(is_collection),
    m_name(name),
    m_buffers(buffers) { }

remote_resource_info::~remote_resource_info() { }

resource_type 
remote_resource_info::type() const {
    return resource_type::remote_resource;
}

std::string
remote_resource_info::address() const {
    return m_address;
}

std::string 
remote_resource_info::nsid() const {
    return m_nsid;
}

bool 
remote_resource_info::is_remote() const {
    return true;
}

std::string 
remote_resource_info::to_string() const {
    return "REMOTE_RESOURCE[" + m_nsid + "@" + m_address + ":" + m_name + "]";
}

std::string
remote_resource_info::name() const {
    return m_name;
}

bool
remote_resource_info::is_collection() const {
    return m_is_collection;
}

bool
remote_resource_info::has_buffers() const {
    return m_buffers.count() != 0;
}

hermes::exposed_memory
remote_resource_info::buffers() const {
    return m_buffers;
}

} // namespace detail
} // namespace data
} // namespace norns
