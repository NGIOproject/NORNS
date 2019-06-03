#ifndef __SHARED_PATH_IMPL_HPP__
#define __SHARED_PATH_IMPL_HPP__

#include <string>
#include <memory>
#include <vector>
#include <system_error>
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

namespace norns {

// forward declarations
namespace storage {
class posix_filesystem;
}

namespace data {

enum class resource_type;

namespace detail {

template <>
struct resource_impl<resource_type::shared_posix_path> : public resource {

    resource_impl(const std::shared_ptr<const storage::backend> parent, 
                  const bfs::path& name);

    std::string name() const override final;
    resource_type type() const override final;
    bool is_collection() const override final;
    std::size_t packed_size() const override final;
    const std::shared_ptr<const storage::backend> parent() const override final;
    std::string to_string() const override final;

    const bfs::path m_name_in_namespace; // absolute pathname w.r.t. backend's mount point
    const bfs::path m_canonical_path; // canonical pathname
    const bool m_is_collection;
    const std::shared_ptr<const storage::posix_filesystem> m_parent;
};

} // namespace detail
} // namespace data
} // namespace norns

#endif /* __SHARED_PATH_IMPL_HPP__ */
