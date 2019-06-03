#ifndef __MEMORY_BUFFER_HPP__
#define __MEMORY_BUFFER_HPP__

#include "resource-type.hpp"
#include "resource.hpp"
#include "detail/memory-region-info.hpp"
#include "detail/memory-region-impl.hpp"

namespace norns {
namespace data {

/* aliases for convenience */
using memory_buffer = detail::memory_region_info;
using memory_region_info = detail::memory_region_info;
using memory_region_resource = detail::resource_impl<resource_type::memory_region>;

} // namespace data
} // namespace norns

#endif /* __MEMORY_BUFFER_HPP__ */
