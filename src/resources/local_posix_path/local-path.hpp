#ifndef __LOCAL_PATH_HPP__
#define __LOCAL_PATH_HPP__

#include "resource-type.hpp"
#include "resource.hpp"

#include "detail/local-path-info.hpp"
#include "detail/local-path-impl.hpp"

namespace norns {
namespace data {

using local_path_info = detail::local_path_info;
using local_path_resource = detail::resource_impl<resource_type::local_posix_path>;

} // namespace data
} // namespace norns

#endif /* __LOCAL_PATH_HPP__ */
