#include "common.hpp"
#include "utils.hpp"
#include "resource-type.hpp"
#include "resource.hpp"
#include "memory-region-info.hpp"
#include "memory-region-impl.hpp"
#include "backends/process-memory.hpp"

namespace norns {
namespace data {
namespace detail {

// local alias for convenience
using memory_region_resource = resource_impl<resource_type::memory_region>;

memory_region_resource::resource_impl(const std::shared_ptr<const storage::backend> parent,
                                      const uint64_t address, const std::size_t size) :
    m_address(address),
    m_size(size),
    m_parent(std::static_pointer_cast<const storage::detail::process_memory>(std::move(parent))) { }

std::string memory_region_resource::name() const {
    return ""; // TODO?
}

resource_type memory_region_resource::type() const {
    return resource_type::memory_region;
}

bool memory_region_resource::is_collection() const {
    return false;
}

std::size_t
memory_region_resource::packed_size() const {
    return m_size;
}

const std::shared_ptr<const storage::backend>
memory_region_resource::parent() const {
    return std::static_pointer_cast<const storage::backend>(m_parent);
}

uint64_t memory_region_resource::address() const {
    return m_address;
}

std::size_t memory_region_resource::size() const {
    return m_size;
}

std::string memory_region_resource::to_string() const {
    return utils::n2hexstr(m_address) + std::to_string(m_size);
}

} // namespace detail
} // namespace data
} // namespace norns
