#include "logger.hpp"
#include "resources.hpp"
#include "auth.hpp"
#include "io/task-info.hpp"
#include "memory-to-shared-path.hpp"

namespace norns {
namespace io {

memory_region_to_shared_path_transferor::
    memory_region_to_shared_path_transferor(const context &ctx) :
        m_ctx(ctx) { }

bool 
memory_region_to_shared_path_transferor::validate(
        const std::shared_ptr<data::resource_info>& src_info,
        const std::shared_ptr<data::resource_info>& dst_info) const {

    const auto& d_src = reinterpret_cast<const data::memory_region_info&>(*src_info);
    const auto& d_dst = reinterpret_cast<const data::shared_path_info&>(*dst_info);

    (void) d_src;
    (void) d_dst;

    LOGGER_WARN("Validation not implemented");

    return true;
}

std::error_code 
memory_region_to_shared_path_transferor::transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<const data::resource>& src,  
        const std::shared_ptr<const data::resource>& dst) const {

    const auto& d_src = reinterpret_cast<const data::memory_region_resource&>(*src);
    const auto& d_dst = reinterpret_cast<const data::shared_path_resource&>(*dst);

    (void) auth;
    (void) task_info;
    (void) d_src;
    (void) d_dst;

    LOGGER_WARN("transfer not implemented");

    return std::make_error_code(static_cast<std::errc>(0));
}

std::error_code 
memory_region_to_shared_path_transferor::accept_transfer(
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
memory_region_to_shared_path_transferor::to_string() const {
    return "transferor[memory_region => shared_path]";
}

} // namespace io
} // namespace norns
