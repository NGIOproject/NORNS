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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <system_error>
#include <boost/filesystem.hpp>

#include "logger.hpp"
#include "backends.hpp"
#include "local-path.hpp"

namespace bfs = boost::filesystem;

namespace data {

/*! Remote path data */
local_path::local_path(std::string nsid, std::string datapath)
    : m_nsid(nsid),
      m_datapath(datapath) {}

local_path::~local_path() { }

resource_type local_path::type() const {
    return resource_type::local_posix_path;
}

std::string local_path::nsid() const {
    return m_nsid;
}

bool local_path::is_remote() const {
    return false;
}

std::string local_path::to_string() const {
    return "LOCAL_PATH[\"" + m_nsid + "\", \"" + m_datapath + "\"]";
}

std::string local_path::datapath() const {
    return m_datapath;
}


namespace detail {

/* Specific resource implementation */
local_path_resource::resource_impl(std::shared_ptr<resource_info> base_info) :
    m_backend(),
    m_resource_info(std::static_pointer_cast<local_path>(base_info)) { }

std::string local_path_resource::to_string() const {
    return m_backend->to_string() + m_resource_info->to_string();
}

// resource_type local_path_resource::type() const {
//     return resource_type::local_posix_path;
// }

std::shared_ptr<resource_info> local_path_resource::info() const {
    return m_resource_info;
}

std::shared_ptr<storage::backend> local_path_resource::backend() const {
    return m_backend;
}

void local_path_resource::set_backend(const std::shared_ptr<storage::backend> backend) {
    m_backend = backend;
}

/* Specific stream implementation */
local_path_stream::stream_impl(std::shared_ptr<resource> resource, stream_type type) {

    // downcast generic resource_info to specific implementation
    auto info = std::static_pointer_cast<local_path>(resource->info());

    bfs::path mount_point = bfs::canonical(resource->backend()->mount());
    bfs::path rsrc_path = info->datapath();

    bfs::path rsrc_abs_path = mount_point / rsrc_path;

//    LOGGER_ERROR("rsrc_abs_path: {}", rsrc_abs_path);

//    if(info->is_directory()) { <--- we can't know unless the user provides it
//    }
//    else {
//      ... // code below
//    }

    bfs::path filename = rsrc_abs_path.filename();
    bfs::path parents = rsrc_abs_path.parent_path();

    switch(type) {
        case stream_type::input:
            m_fd = open(rsrc_abs_path.c_str(), O_RDONLY);
            break;
        case stream_type::output:
            m_fd = open(rsrc_abs_path.c_str(), O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR);
            break;
    }

    if(m_fd == -1) {
        throw std::system_error(errno, std::generic_category());
    }
}

local_path_stream::~local_path_stream() {
    if(m_fd != -1) {
        if(close(m_fd) == -1) {
            LOGGER_ERROR("Error when closing stream's file descriptor: {}", strerror(errno));
        }
    }
}

std::size_t local_path_stream::read(buffer& b) {

    if(m_fd == -1) {
        throw std::system_error(EBADF, std::generic_category());
    }

    size_t size = b.size(); // max buffer size
    size_t brecvd = 0; // bytes read
    size_t bleft = size; // bytes left to read
    ssize_t n = 0;

    while(brecvd < size) {
        n = ::read(m_fd, &b[0] + brecvd, bleft);

        if(n == -1 || n == 0) {
            if(errno == EINTR) {
                continue;
            }
            break;
        }

        brecvd += n;
        bleft -= n;
    }

    if(n == -1) {
        throw std::system_error(errno, std::generic_category());
    }

    if(brecvd < size) {
        b.resize(brecvd);
    }

    return brecvd;
}

std::size_t local_path_stream::write(const buffer& b) {

    if(m_fd == -1) {
        throw std::system_error(EBADF, std::generic_category());
    }

    size_t size = b.size();
    size_t bsent = 0; // bytes written
    size_t bleft = size; // bytes left to write
    ssize_t n = 0;

    while(bsent < size) {
        n = ::write(m_fd, &b[0] + bsent, bleft);

        if(n == -1) {
            if(errno == EINTR) {
                continue;
            }
            break;
        }

        bsent += n;
        bleft -= n;
    }

    if(n == -1) {
        throw std::system_error(errno, std::generic_category());
    }

    return bsent;
}

} // namespace detail

} // namespace data
