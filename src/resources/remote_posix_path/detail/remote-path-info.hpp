#ifndef __REMOTE_PATH_INFO_HPP__
#define __REMOTE_PATH_INFO_HPP__

#include <string>
#include "resource-info.hpp"

namespace norns {
namespace data {

enum class resource_type;

namespace detail {

/*! remote filesystem path data */
struct remote_path_info : public resource_info {

    remote_path_info(const std::string& nsid, const std::string& hostname, 
                     const std::string& datapath);
    ~remote_path_info();
    resource_type type() const override final;
    std::string nsid() const override final;
    bool is_remote() const override final;
    std::string to_string() const override final;

    std::string hostname() const;
    std::string datapath() const;

    std::string m_nsid;
    std::string m_hostname;
    std::string m_datapath;
};

} // namespace detail
} // namespace data
} // namespace norns

#endif /* __REMOTE_PATH_INFO_HPP__ */
