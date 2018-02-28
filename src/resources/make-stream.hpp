/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 *                                                                       *
 * This file is part of the Data Scheduler, a daemon for tracking and    *
 * managing requests for asynchronous data transfer in a hierarchical    *
 * storage environment.                                                  *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The Data Scheduler is free software: you can redistribute it and/or   *
 * modify it under the terms of the GNU Lesser General Public License    *
 * as published by the Free Software Foundation, either version 3 of     *
 * the License, or (at your option) any later version.                   *
 *                                                                       *
 * The Data Scheduler is distributed in the hope that it will be useful, *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with Data Scheduler.  If not, see                *
 * <http://www.gnu.org/licenses/>.                                       *
 *                                                                       *
 *************************************************************************/

#ifndef __MAKE_STREAM_HPP__
#define __MAKE_STREAM_HPP__

// add additional concrete implementations here
#include "resources/local-path.hpp"
#include "resources/memory-buffer.hpp"
#include "resources/shared-path.hpp"
#include "resources/remote-path.hpp"

namespace data {

inline std::shared_ptr<stream> make_stream(std::shared_ptr<resource> rsrc) {
    switch(rsrc->type()) {
        case data::resource_type::memory_region:
            return std::make_shared<data::memory_region_stream>(rsrc);
        case data::resource_type::local_posix_path:
            return std::make_shared<data::local_path_stream>(rsrc);
        case data::resource_type::shared_posix_path:
            return std::make_shared<data::shared_path_stream>(rsrc);
        case data::resource_type::remote_posix_path:
            return std::make_shared<data::remote_path_stream>(rsrc);
    }

    // return an invalid pointer if type is not known
    return std::shared_ptr<stream>();
}

} // namespace data

#endif /* __MAKE_STREAM_HPP__ */
