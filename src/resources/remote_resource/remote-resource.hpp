#ifndef __REMOTE_RESOURCE_HPP__
#define __REMOTE_RESOURCE_HPP__

#include "resource-type.hpp"
#include "resource.hpp"

#include "detail/remote-resource-info.hpp"
#include "detail/remote-resource-impl.hpp"

namespace norns {
namespace data {

using remote_resource_info = detail::remote_resource_info;
using remote_resource = 
    detail::resource_impl<resource_type::remote_resource>;

} // namespace data
} // namespace norns

#endif // __REMOTE_RESOURCE_HPP__
