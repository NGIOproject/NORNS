#include <system_error>
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

#include "resource-type.hpp"
#include "resource.hpp"
#include "remote-path-info.hpp"
#include "remote-path-impl.hpp"
#include "backends/remote-backend.hpp"

namespace norns {
namespace data {
namespace detail {

// remote alias for convenience
using remote_path_resource = resource_impl<resource_type::remote_posix_path>;

remote_path_resource::resource_impl(
    const std::shared_ptr<const storage::backend> parent)
    : m_parent(std::static_pointer_cast<const storage::detail::remote_backend>(
          std::move(parent))) {}

std::string remote_path_resource::name() const {
    return "PENDING: " + m_parent->to_string();
}

resource_type remote_path_resource::type() const {
    return resource_type::remote_posix_path;
}

bool remote_path_resource::is_collection() const {
    return false;
}

std::size_t
remote_path_resource::packed_size() const {
    return 0;
}

const std::shared_ptr<const storage::backend>
remote_path_resource::parent() const {
    return std::static_pointer_cast<const storage::backend>(m_parent);
}

std::string remote_path_resource::to_string() const {
    return "node42@/foo/bar/baz";
}

} // namespace detail
} // namespace data
} // namespace norns
