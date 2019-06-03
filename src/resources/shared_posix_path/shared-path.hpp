#ifndef __SHARED_PATH_HPP__
#define __SHARED_PATH_HPP__

#include "resource-type.hpp"
#include "resource.hpp"

#include "detail/shared-path-info.hpp"
#include "detail/shared-path-impl.hpp"

namespace norns {
namespace data {

using shared_path_info = detail::shared_path_info;
using shared_path_resource = detail::resource_impl<resource_type::shared_posix_path>;

} // namespace data
} // namespace norns

#endif /* __SHARED_PATH_HPP__ */
