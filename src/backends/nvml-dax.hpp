#ifndef __NVML_DAX_HPP__
#define __NVML_DAX_HPP__

#include <system_error>
#include <boost/filesystem.hpp>

#include "backend-base.hpp"

namespace bfs = boost::filesystem;

namespace norns {
namespace storage {

class nvml_dax final : public storage::backend {
public:
    nvml_dax(const std::string& nsid, bool track, const bfs::path& mount, uint32_t quota);

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
    std::string to_string() const final;

private:
    std::string m_nsid;
    bool        m_track;
    bfs::path   m_mount;
    uint32_t    m_quota;
};

//NORNS_REGISTER_BACKEND(backend_type::nvml, nvml_dax);

} // namespace storage
} // namespace norns


#endif // __NVML_DAX_HPP__
