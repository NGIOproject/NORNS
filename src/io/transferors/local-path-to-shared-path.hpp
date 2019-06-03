#ifndef __IO_LOCAL_PATH_TO_SHARED_PATH_TX__
#define __IO_LOCAL_PATH_TO_SHARED_PATH_TX__

#include <memory>
#include <system_error>
#include "context.hpp"
#include "transferor.hpp"

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

struct local_path_to_shared_path_transferor : public transferor {

    local_path_to_shared_path_transferor(const context& ctx);

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
    context m_ctx;
};

} // namespace io
} // namespace norns

#endif /* __LOCAL_PATH_TO_SHARED_PATH_TX__ */
