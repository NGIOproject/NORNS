#ifndef __REMOTE_PATH_IMPL_HPP__
#define __REMOTE_PATH_IMPL_HPP__

#include <string>
#include <memory>

namespace norns {

// forward declarations
namespace storage { namespace detail {
class remote_backend;
}}

namespace data {

enum class resource_type;

namespace detail {

template <>
struct resource_impl<resource_type::remote_posix_path> : public resource {

    resource_impl(const std::shared_ptr<const storage::backend> parent);

    std::string name() const override final;
    resource_type type() const override final;
    bool is_collection() const override final;
    std::size_t packed_size() const override final;
    const std::shared_ptr<const storage::backend> parent() const override final;
    std::string to_string() const override final;


    const bfs::path m_name_in_namespace;
    const std::shared_ptr<const storage::detail::remote_backend> m_parent;
};

} // namespace detail
} // namespace data
} // namespace norns

#endif /* __REMOTE_PATH_IMPL_HPP__ */
