#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "utils.hpp"
#include "logger.hpp"
#include "resources.hpp"
#include "auth.hpp"
#include "io/task-info.hpp"
#include "backends/posix-fs.hpp"
#include "local-path-to-shared-path.hpp"

namespace norns {
namespace io {

local_path_to_shared_path_transferor::local_path_to_shared_path_transferor(
    const context& ctx) :
        m_ctx(ctx) { }

bool 
local_path_to_shared_path_transferor::validate(
        const std::shared_ptr<data::resource_info>& src_info,
        const std::shared_ptr<data::resource_info>& dst_info) const {

    (void) src_info;
    (void) dst_info;

    LOGGER_WARN("Validation not implemented");

    return true;
}

std::error_code 
local_path_to_shared_path_transferor::transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<const data::resource>& src,  
        const std::shared_ptr<const data::resource>& dst) const {

    (void) auth;
    (void) task_info;
    (void) src;
    (void) dst;

    LOGGER_WARN("transfer not implemented");

    return std::make_error_code(static_cast<std::errc>(0));
}

std::error_code 
local_path_to_shared_path_transferor::accept_transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<const data::resource>& src,  
        const std::shared_ptr<const data::resource>& dst) const {

    (void) auth;
    (void) task_info;
    (void) src;
    (void) dst;

    LOGGER_ERROR("This function should never be called for this transfer type");
    return std::make_error_code(static_cast<std::errc>(0));
}

std::string 
local_path_to_shared_path_transferor::to_string() const {
    return "transferor[local_path => shared_path]";
}

} // namespace io
} // namespace norns
