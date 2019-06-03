#include <sstream>
#include "resource-type.hpp"
#include "resource-info.hpp"
#include "memory-region-info.hpp"

namespace norns {
namespace data {
namespace detail {

/*! Remote path data */
memory_region_info::memory_region_info(uint64_t address, std::size_t size)
    : m_address(address),
      m_size(size) {}

memory_region_info::~memory_region_info() { }

resource_type memory_region_info::type() const {
    return resource_type::memory_region;
}

std::string memory_region_info::nsid() const {
    // TODO: define and use constants
    return "[[internal::memory]]";
}

bool memory_region_info::is_remote() const {
    return false;
}

std::string memory_region_info::to_string() const {
    std::stringstream ss;
    ss << "0x" << std::hex << m_address << "+" << "0x" << m_size;
    return "MEMBUF[" + ss.str() + "]";
}

uint64_t memory_region_info::address() const {
    return m_address;
}

std::size_t memory_region_info::size() const {
    return m_size;
}

} // namespace detail
} // namespace data
} // namespace norns
