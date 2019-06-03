#include <system_error>
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

#include "common.hpp"
#include "resource-type.hpp"
#include "resource.hpp"
#include "local-path-info.hpp"
#include "local-path-impl.hpp"
#include "backends/posix-fs.hpp"
#include "utils.hpp"
#include "logger.hpp"

namespace norns {
namespace data {
namespace detail {

// local alias for convenience
using local_path_resource = resource_impl<resource_type::local_posix_path>;

local_path_resource::resource_impl(const std::shared_ptr<const storage::backend> parent, 
                                   const bfs::path& name) :
    m_name_in_namespace(name),
    m_canonical_path(parent->mount() / name),
    m_is_collection(bfs::is_directory(m_canonical_path)),
    m_parent(std::static_pointer_cast<const storage::posix_filesystem>(std::move(parent))) { }

std::string
local_path_resource::name() const {
    return m_name_in_namespace.string();
}

resource_type
local_path_resource::type() const {
    return resource_type::local_posix_path;
}

bool
local_path_resource::is_collection() const {
    return m_is_collection;
}

std::size_t
local_path_resource::packed_size() const {
    std::error_code ec;
    boost::system::error_code error;

    std::size_t sz = m_is_collection ?
        utils::tar::estimate_size_once_packed(m_canonical_path, ec) :
        bfs::file_size(m_canonical_path, error);

    return (ec || error) ? 0 : sz;
}

const std::shared_ptr<const storage::backend>
local_path_resource::parent() const {
    return std::static_pointer_cast<const storage::backend>(m_parent);
}

bfs::path local_path_resource::canonical_path() const {
    return m_canonical_path;
}

std::string
local_path_resource::to_string() const {
    return m_canonical_path.string();
}

} // namespace detail
} // namespace data
} // namespace norns
