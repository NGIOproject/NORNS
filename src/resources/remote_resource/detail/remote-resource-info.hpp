#ifndef __REMOTE_RESOURCE_INFO_HPP__
#define __REMOTE_RESOURCE_INFO_HPP__

#include <hermes.hpp>
#include <string>
#include "resource-info.hpp"
#include "rpcs.hpp"

namespace norns {
namespace data {

enum class resource_type;

namespace detail {

/*! Local filesystem path data */
struct remote_resource_info : public resource_info {

    remote_resource_info(const std::string& address,
                         const std::string& nsid, 
                         const std::string& name);

    remote_resource_info(const std::string& address,
                         const std::string& nsid, 
                         bool is_collection,
                         const std::string& name,
                         const hermes::exposed_memory& buffers);
    ~remote_resource_info();
    resource_type type() const override final;
    std::string nsid() const override final;
    bool is_remote() const override final;
    std::string to_string() const override final;

    std::string
    address() const;

    std::string
    name() const;

    bool
    is_collection() const;

    bool
    has_buffers() const;

    hermes::exposed_memory
    buffers() const;

    std::string m_address;
    std::string m_nsid;
    bool m_is_collection;
    std::string m_name;
    hermes::exposed_memory m_buffers;
};

} // namespace detail
} // namespace data
} // namespace norns

#endif // __REMOTE_RESOURCE_INFO_HPP__
