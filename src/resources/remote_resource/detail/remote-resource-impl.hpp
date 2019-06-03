#ifndef __REMOTE_RESOURCE_IMPL_HPP__
#define __REMOTE_RESOURCE_IMPL_HPP__

#include <string>
#include <memory>
#include <vector>
#include <system_error>
#include <hermes.hpp>

namespace norns {

// forward declarations
namespace storage {
namespace detail {
class remote_backend;
}
}

namespace data {

enum class resource_type;

namespace detail {

template <>
struct resource_impl<resource_type::remote_resource> : public resource {

    resource_impl(const std::shared_ptr<const storage::backend> parent, 
                  const std::shared_ptr<const remote_resource_info> rinfo);

    std::string name() const override final;
    resource_type type() const override final;
    bool is_collection() const override final;
    std::size_t packed_size() const override final;
    const std::shared_ptr<const storage::backend> parent() const override final;
    std::string to_string() const override final;

    std::string
    address() const;

    std::string
    nsid() const;

    bool
    has_buffer() const;

    hermes::exposed_memory
    buffers() const;

    const std::string m_name;
    const std::string m_address;
    const std::string m_nsid;
    const hermes::exposed_memory m_buffers;
    const bool m_is_collection;
    const std::shared_ptr<const storage::detail::remote_backend> m_parent;
};

} // namespace detail
} // namespace data
} // namespace norns

#endif /* __REMOTE_RESOURCE_IMPL_HPP__ */
