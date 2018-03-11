/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of the NORNS Data Scheduler, a service that allows  *
 * other programs to start, track and manage asynchronous transfers of   *
 * data resources transfers requests between different storage backends. *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The NORNS Data Scheduler is free software: you can redistribute it    *
 * and/or modify it under the terms of the GNU Lesser General Public     *
 * License as published by the Free Software Foundation, either          *
 * version 3 of the License, or (at your option) any later version.      *
 *                                                                       *
 * The NORNS Data Scheduler is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with the NORNS Data Scheduler.  If not, see      *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

#ifndef __MAKE_RESOURCE_HPP__
#define __MAKE_RESOURCE_HPP__

// add additional concrete implementations here
#include "resources/local-path.hpp"
#include "resources/memory-buffer.hpp"
#include "resources/shared-path.hpp"
#include "resources/remote-path.hpp"

namespace norns {
namespace data {

inline std::shared_ptr<resource> make_resource(std::shared_ptr<resource_info> rinfo) {
    switch(rinfo->type()) {
        case data::resource_type::memory_region:
            return std::make_shared<data::memory_region_resource>(rinfo);
        case data::resource_type::local_posix_path:
            return std::make_shared<data::local_path_resource>(rinfo);
        case data::resource_type::shared_posix_path:
            return std::make_shared<data::shared_path_resource>(rinfo);
        case data::resource_type::remote_posix_path:
            return std::make_shared<data::remote_path_resource>(rinfo);
    }

    // return an invalid pointer if type is not known
    return std::shared_ptr<resource>();
}

} // namespace data
} // namespace norns

#endif /* __MAKE_RESOURCE_HPP__ */
