#ifndef __IO_LOCAL_PATH_TO_REMOTE_RESOURCE_TX__
#define __IO_LOCAL_PATH_TO_REMOTE_RESOURCE_TX__

#include <memory>
#include <system_error>
#include "context.hpp"
#include "transferor.hpp"

namespace hermes {
class async_engine;

template <typename T> class request;

} // namespace hermes

namespace norns {

// forward declarations
namespace auth {
struct credentials;
}

namespace data {
struct resource_info;
struct resource;
}

namespace io {

struct local_path_to_remote_resource_transferor : public transferor {

    local_path_to_remote_resource_transferor(const context& ctx);

    bool 
    validate(const std::shared_ptr<data::resource_info>& src_info,
             const std::shared_ptr<data::resource_info>& dst_info) 
        const override final;

    std::error_code 
    transfer(const auth::credentials& auth,                
             const std::shared_ptr<task_info>& task_info,
             const std::shared_ptr<const data::resource>& src,  
             const std::shared_ptr<const data::resource>& dst) 
        const override final;

    std::error_code 
    accept_transfer(const auth::credentials& auth,                
                const std::shared_ptr<task_info>& task_info,
                const std::shared_ptr<const data::resource>& src,  
                const std::shared_ptr<const data::resource>& dst) 
        const override final;

    std::string 
    to_string() const override final;

private:
    bfs::path m_staging_directory;
    std::shared_ptr<hermes::async_engine> m_network_service;

};

} // namespace io
} // namespace norns

#endif /* __LOCAL_PATH_TO_REMOTE_RESOURCE_TX__ */
