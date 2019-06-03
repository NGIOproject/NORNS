#ifndef __LOCAL_PATH_INFO_HPP__
#define __LOCAL_PATH_INFO_HPP__

#include <string>
#include "resource-info.hpp"

namespace norns {
namespace data {

enum class resource_type;

namespace detail {

/*! Local filesystem path data */
struct local_path_info : public resource_info {

    local_path_info(std::string nsid, std::string datapath);
    ~local_path_info();
    resource_type type() const override final;
    std::string nsid() const override final;
    bool is_remote() const override final;
    std::string to_string() const override final;

    std::string datapath() const;

    std::string m_nsid;
    std::string m_datapath;
};

} // namespace detail
} // namespace data
} // namespace norns

#endif /* __LOCAL_PATH_INFO_HPP__ */
