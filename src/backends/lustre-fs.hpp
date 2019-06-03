#ifndef __LUSTRE_FS_HPP__
#define __LUSTRE_FS_HPP__

#include <system_error>
#include <boost/filesystem.hpp>

//#include "norns/norns_backends.h"
#include "backend-base.hpp"

namespace bfs = boost::filesystem;

namespace norns {
namespace storage {

class lustre final : public storage::backend {
public:
    lustre(const std::string& nsid, bool track, const bfs::path& mount, uint32_t quota);

    std::string nsid() const override final;
    bool is_tracked() const override final;
    bool is_empty() const override final;
    bfs::path mount() const override final;
    uint32_t quota() const override final;

    resource_ptr new_resource(const resource_info_ptr& rinfo, bool is_collection, std::error_code& ec) const override final;
    resource_ptr get_resource(const resource_info_ptr& rinfo, std::error_code& ec) const override final;
    void remove(const resource_info_ptr& rinfo, std::error_code& ec) const override final;
    std::size_t get_size(const resource_info_ptr& rinfo, std::error_code& ec) const override final;

    bool accepts(resource_info_ptr res) const override final;
    std::string to_string() const override final;

protected:
    std::string m_nsid;
    bool        m_track;
    bfs::path   m_mount;
    uint32_t    m_quota;
};

//NORNS_REGISTER_BACKEND(backend_type::lustre, lustre);

} // namespace storage
} // namespace norns

#endif // __LUSTRE_FS_HPP__
