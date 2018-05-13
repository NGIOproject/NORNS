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

#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "utils.hpp"
#include "logger.hpp"
#include "resources.hpp"
#include "auth/process-credentials.hpp"
#include "backends/posix-fs.hpp"
#include "local-path-to-local-path.hpp"

namespace {

ssize_t
get_filesize(int fd) {
	struct stat st;

	if(::fstat(fd, &st) != -1) {
        return static_cast<ssize_t>(st.st_size);
    }

    return static_cast<ssize_t>(-1);
}

ssize_t
do_sendfile(int in_fd, int out_fd) {
	ssize_t sz = ::get_filesize(in_fd);

    // provide kernel with advices on how we are going to use the data
    if(::posix_fadvise(in_fd, 0, sz, POSIX_FADV_WILLNEED) != 0) {
        return static_cast<ssize_t>(-1);
    }

    if(::posix_fadvise(in_fd, 0, sz, POSIX_FADV_SEQUENTIAL) != 0) {
        return static_cast<ssize_t>(-1);
    }

    // preallocate output file
    if(::fallocate(out_fd, 0, 0, sz) == -1) {
        // filesystem doesn't support fallocate(), fallback to truncate()
        if(errno == EOPNOTSUPP) {
            if(::ftruncate(out_fd, sz) != 0) {
                return static_cast<ssize_t>(-1);
            }
        }
        return static_cast<ssize_t>(-1);
    }

    // copy data
	off_t offset = 0;
	while(offset < sz) {
		if(::sendfile(out_fd, in_fd, &offset, sz - offset) == -1) {
			if(errno != EINTR) {
                return static_cast<ssize_t>(-1);
			}
		}
	}

	return sz;
}

std::error_code
copy_file(const bfs::path& src,  const bfs::path& dst) {

    const char* src_path = src.c_str();
    const char* dst_name = dst.c_str();

    int in_fd = ::open(src_path, O_RDONLY);

    if(in_fd == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    int out_fd = ::open(dst_name, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

    if(out_fd == -1) {
        close(in_fd);
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    if(do_sendfile(in_fd, out_fd) == -1) {
        close(in_fd);
        close(out_fd);
        return std::make_error_code(static_cast<std::errc>(errno));
    }


    for(auto fd : {in_fd, out_fd}) {
retry_close:
        if(close(fd) == -1) {
            if(errno == EINTR) {
                goto retry_close;
            }

            return std::make_error_code(static_cast<std::errc>(errno));
        }
    }

    return std::make_error_code(static_cast<std::errc>(0));
}

std::error_code
copy_directory(const bfs::path& src, const bfs::path& dst) {

    boost::system::error_code ec;
    auto it = bfs::recursive_directory_iterator(src, ec);
    const auto end = bfs::recursive_directory_iterator();

    if(ec) {
        return std::make_error_code(static_cast<std::errc>(ec.value()));
    }

    for(; it != end; ++it) {
        const auto dst_path = dst / bfs::relative(src, *it);

        if(bfs::is_directory(*it)) {
            if(!bfs::exists(dst_path)) {

                bfs::create_directory(dst_path, ec);

                if(ec) {
                    return std::make_error_code(static_cast<std::errc>(ec.value()));
                }
            }
            continue;
        }

        if(auto err = ::copy_file(*it, dst_path)) {
            return err;
        }
    }

    return std::make_error_code(static_cast<std::errc>(0));
}


} // namespace

namespace norns {
namespace io {

bool 
local_path_to_local_path_transferor::validate(
        const std::shared_ptr<data::resource_info>& src_info,
        const std::shared_ptr<data::resource_info>& dst_info) const {

    (void) src_info;
    (void) dst_info;

    LOGGER_WARN("Validation not implemented");

    return true;
}

std::error_code 
local_path_to_local_path_transferor::transfer(
        const auth::credentials& usr_creds, 
        const std::shared_ptr<const data::resource>& src,  
        const std::shared_ptr<const data::resource>& dst) const {

    (void) usr_creds;

    const auto& d_src = 
        reinterpret_cast<const data::local_path_resource&>(*src);
    const auto& d_dst = 
        reinterpret_cast<const data::local_path_resource&>(*dst);

    LOGGER_DEBUG("transfer: {} -> {}", 
            d_src.canonical_path(), d_dst.canonical_path());

    if(bfs::is_directory(d_src.canonical_path())) {
        return ::copy_directory(d_src.canonical_path(), 
                                d_dst.canonical_path());
    }

    return ::copy_file(d_src.canonical_path(), d_dst.canonical_path());
}


} // namespace io
} // namespace norns
