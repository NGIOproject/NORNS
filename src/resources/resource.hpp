#ifndef __RESOURCE_HPP__
#define __RESOURCE_HPP__

#include <memory>
#include <vector>

namespace norns {

// forward declare storage::backend
namespace storage {
    class backend;
}

namespace data {

enum class resource_type;

/*! A fully qualified resource for which we have a description and the backend
 * that stores it */
struct resource : public std::enable_shared_from_this<resource> {
    virtual ~resource() {}
    virtual resource_type type() const = 0;
    virtual std::string name() const = 0;
    virtual bool is_collection() const = 0;
    virtual std::size_t packed_size() const = 0;
    virtual const std::shared_ptr<const storage::backend> parent() const = 0; 
    virtual std::string to_string() const = 0;
};

namespace detail {

/*! Generic template for resource implementations 
 * (a concrete specialization must be provided for each resource_type) */
template <resource_type RT>
struct resource_impl;

} // namespace detail
} // namespace data 
} // namespace norns

#endif /* __RESOURCE_HPP__ */
