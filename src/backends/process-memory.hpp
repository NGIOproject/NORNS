#ifndef __PROCESS_MEMORY_HPP__
#define __PROCESS_MEMORY_HPP__

#include <system_error>
#include <boost/filesystem.hpp>

#include "backend-base.hpp"

namespace bfs = boost::filesystem;

namespace norns {
namespace storage {
namespace detail {

class process_memory final : public storage::backend {
public:
    process_memory(const std::string& nsid);

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

private:
    std::string m_nsid;
};

// no need to register it since it's never going to be created 
// automatically using the factory 
//NORNS_REGISTER_BACKEND(backend_type::process_memory, process_memory);

} // namespace detail
} // namespace storage
} // namespace norns


#endif /* __PROCESS_MEMORY_HPP__ */

