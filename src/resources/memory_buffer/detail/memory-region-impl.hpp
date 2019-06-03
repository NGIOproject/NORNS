#ifndef __MEMORY_REGION_IMPL_HPP__
#define __MEMORY_REGION_IMPL_HPP__

#include <string>
#include <memory>

namespace norns {

// forward declarations
namespace storage { namespace detail {
class process_memory;
}}

namespace data {

enum class resource_type;

namespace detail {

template <>
struct resource_impl<resource_type::memory_region> : public resource {

    resource_impl(const std::shared_ptr<const storage::backend> parent,
                  const uint64_t address, const std::size_t size);
    std::string name() const override final;
    resource_type type() const override final;
    bool is_collection() const override final;
    std::size_t packed_size() const override final;
    const std::shared_ptr<const storage::backend> parent() const override final;
    std::string to_string() const override final;

    uint64_t address() const;
    std::size_t size() const;

    const uint64_t m_address;
    const std::size_t m_size;
    const std::shared_ptr<const storage::detail::process_memory> m_parent;
};

} // namespace detail
} // namespace data
} // namespace norns

#endif /* __MEMORY_REGION_IMPL_HPP__ */
