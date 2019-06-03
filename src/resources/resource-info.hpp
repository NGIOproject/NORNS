#ifndef __RESOURCE_INFO_HPP__
#define __RESOURCE_INFO_HPP__

#include <memory>
#include <vector>

namespace norns {
namespace data {

/*! Base class for information about data resources */
struct resource_info {
    virtual ~resource_info() {}
    virtual resource_type type() const = 0;
    virtual std::string nsid() const = 0;
    virtual bool is_remote() const = 0;
    virtual std::string to_string() const = 0;
};

} // namespace data 
} // namespace norns

#endif /* __RESOURCE_INFO_HPP__ */

