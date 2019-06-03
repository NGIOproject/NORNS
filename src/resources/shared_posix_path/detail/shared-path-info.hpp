#ifndef __SHARED_PATH_INFO_HPP__
#define __SHARED_PATH_INFO_HPP__

#include <string>
#include "resource-info.hpp"

namespace norns {
namespace data {

namespace detail {

/*! Local filesystem path data */
struct shared_path_info : public resource_info {

    shared_path_info(const std::string& nsid, const std::string& datapath);
    ~shared_path_info();
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

#endif /* __SHARED_PATH_INFO_HPP__ */
