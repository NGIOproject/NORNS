#include <boost/filesystem.hpp>
#include "logger.hpp"
#include "backend-base.hpp"

namespace bfs = boost::filesystem;

namespace norns {
namespace storage {

backend_factory& 
backend_factory::get() {
    static backend_factory _;
    return _;
}

std::shared_ptr<backend>
backend_factory::create(const backend_type type, 
                        const std::string& nsid, 
                        bool track, 
                        const bfs::path& mount, 
                        uint32_t quota) const {

    boost::system::error_code ec;

    bfs::path canonical_mount = bfs::canonical(mount, ec);

    if(ec) {
        LOGGER_ERROR("Invalid mount point: {}", ec.message());
        throw std::invalid_argument("");
    }

    const int32_t id = static_cast<int32_t>(type);

    const auto& it = m_registrar.find(id);

    if(it != m_registrar.end()){
        return std::shared_ptr<backend>(
                it->second(nsid, track, canonical_mount, quota));
    }
    else{
        throw std::invalid_argument("Unrecognized backend type!");
    }
}

} // namespace storage
} // namespace norns
