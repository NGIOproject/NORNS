#ifndef __REMOTE_PATH_HPP__
#define __REMOTE_PATH_HPP__

#include "resource-type.hpp"
#include "resource.hpp"

#include "detail/remote-path-info.hpp"
#include "detail/remote-path-impl.hpp"

namespace norns {
namespace data {

using remote_path_info = detail::remote_path_info;
using remote_path_resource = detail::resource_impl<resource_type::remote_posix_path>;

} // namespace data
} // namespace norns

#endif /* __REMOTE_PATH_HPP__ */
