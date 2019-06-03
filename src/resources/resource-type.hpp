#ifndef __RESOURCE_TYPE_HPP__
#define __RESOURCE_TYPE_HPP__

#include <string>

namespace norns {
namespace data {

/*! Supported resource types */
enum class resource_type {
    memory_region,
    local_posix_path,
    shared_posix_path,
    remote_posix_path,
    remote_resource,
    ignorable
};

} // namespace data

namespace utils {

static inline std::string to_string(data::resource_type type) {
    switch(type) {
        case data::resource_type::memory_region:
            return "MEMORY_REGION";
        case data::resource_type::local_posix_path:
            return "LOCAL_PATH";
        case data::resource_type::shared_posix_path:
            return "SHARED_PATH";
        case data::resource_type::remote_posix_path:
            return "REMOTE_PATH";
        case data::resource_type::remote_resource:
            return "REMOTE_RESOURCE";
        default:
            return "UNKNOWN_RESOURCE_TYPE";
    }
}

}

} // namespace norns

#endif /* __RESOURCE_TYPE_HPP__ */
