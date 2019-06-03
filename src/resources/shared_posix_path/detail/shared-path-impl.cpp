#include <system_error>
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

#include "resource-type.hpp"
#include "resource.hpp"
#include "shared-path-info.hpp"
#include "shared-path-impl.hpp"
#include "backends/posix-fs.hpp"

namespace norns {
namespace data {
namespace detail {

// local alias for convenience
using shared_path_resource = resource_impl<resource_type::shared_posix_path>;

shared_path_resource::resource_impl(const std::shared_ptr<const storage::backend> parent, 
                                    const bfs::path& name) :
    m_name_in_namespace(name),
    m_canonical_path(parent->mount() / name),
    m_is_collection(bfs::is_directory(m_canonical_path)),
    m_parent(std::static_pointer_cast<const storage::posix_filesystem>(std::move(parent))) { }

std::string shared_path_resource::name() const {
    return m_name_in_namespace.string();
}

resource_type shared_path_resource::type() const {
    return resource_type::shared_posix_path;
}

bool shared_path_resource::is_collection() const {
    return m_is_collection;
}

std::size_t
shared_path_resource::packed_size() const {
    return 0;
}

const std::shared_ptr<const storage::backend>
shared_path_resource::parent() const {
    return std::static_pointer_cast<const storage::backend>(m_parent);
}

std::string shared_path_resource::to_string() const {
    return "/foo/bar/baz"; // TODO
}

} // namespace detail
} // namespace data
} // namespace norns
