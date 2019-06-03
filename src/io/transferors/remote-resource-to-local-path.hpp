#ifndef __IO_REMOTE_RESOURCE_TO_LOCAL_PATH_TX__
#define __IO_REMOTE_RESOURCE_TO_LOCAL_PATH_TX__

#include <string>
#include <memory>

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

struct remote_resource_to_local_path_transferor : public transferor {

    remote_resource_to_local_path_transferor(const context& ctx);

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

#endif // __IO_REMOTE_RESOURCE_TO_LOCAL_PATH_TX__
