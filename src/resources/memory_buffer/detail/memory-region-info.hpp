#ifndef __MEMORY_REGION_INFO_HPP__
#define __MEMORY_REGION_INFO_HPP__

#include <string>
#include "resource-info.hpp"

namespace norns {
namespace data {

enum class resource_type;

namespace detail {

/*! Local filesystem path data */
struct memory_region_info : public resource_info {

    memory_region_info(uint64_t address, std::size_t size);
    ~memory_region_info();
    resource_type type() const override final;
    std::string nsid() const override final;
    bool is_remote() const override final;
    std::string to_string() const override final;

    uint64_t address() const;
    std::size_t size() const;

    uint64_t m_address;
    std::size_t m_size;
};

} // namespace detail
} // namespace data
} // namespace norns

#endif /* __MEMORY_REGION_INFO_HPP__ */
