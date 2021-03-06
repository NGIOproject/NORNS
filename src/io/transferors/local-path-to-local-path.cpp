/************************************************************************* 
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of NORNS, a service that allows other programs to   *
 * start, track and manage asynchronous transfers of data resources      *
 * between different storage backends.                                   *
 *                                                                       *
 * See AUTHORS file in the top level directory for information regarding *
 * developers and contributors.                                          *
 *                                                                       *
 * This software was developed as part of the EC H2020 funded project    *
 * NEXTGenIO (Project ID: 671951).                                       *
 *     www.nextgenio.eu                                                  *
 *                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining *
 * a copy of this software and associated documentation files (the       *
 * "Software"), to deal in the Software without restriction, including   *
 * without limitation the rights to use, copy, modify, merge, publish,   *
 * distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to *
 * the following conditions:                                             *
 *                                                                       *
 * The above copyright notice and this permission notice shall be        *
 * included in all copies or substantial portions of the Software.       *
 *                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                 *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS   *
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN    *
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN     *
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE      *
 * SOFTWARE.                                                             *
 *************************************************************************/

#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <climits>
#include "config.h"

#include "utils.hpp"
#include "logger.hpp"
#include "resources.hpp"
#include "auth.hpp"
#include "io/task-info.hpp"
#include "backends/posix-fs.hpp"
#include "local-path-to-local-path.hpp"
#include <iostream>

namespace {

ssize_t
get_filesize(int fd) {
	struct stat st;

	if(::fstat(fd, &st) != -1) {
        return static_cast<ssize_t>(st.st_size);
    }

    LOGGER_ERROR("fstat() failed: {}", 
            std::make_error_code(static_cast<std::errc>(errno)).message());

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
#ifdef HAVE_FALLOCATE
    if(::fallocate(out_fd, 0, 0, sz) == -1) {
        if(errno != EOPNOTSUPP) {
            return static_cast<ssize_t>(-1);
        }
#endif // HAVE_FALLOCATE

        // filesystem doesn't support fallocate(), fallback to truncate()
        if(::ftruncate(out_fd, sz) != 0) {
            return static_cast<ssize_t>(-1);
        }

#ifdef HAVE_FALLOCATE
    }
#endif // HAVE_FALLOCATE

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
copy_file(const std::shared_ptr<norns::io::task_info>& task_info, 
          const bfs::path& src, const bfs::path& dst) {

    auto start = std::chrono::steady_clock::now();

    const char* src_path = src.c_str();
    const char* dst_name = dst.c_str();
    ssize_t file_size;

    int in_fd = ::open(src_path, O_RDONLY);

    if(in_fd == -1) {
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    int out_fd = ::open(dst_name, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

    if(out_fd == -1) {
        close(in_fd);
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    if((file_size = do_sendfile(in_fd, out_fd)) == -1) {
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

    double usecs = std::chrono::duration<double, std::micro>(
            std::chrono::steady_clock::now() - start).count();

    task_info->record_transfer(file_size, usecs);

    return std::make_error_code(static_cast<std::errc>(0));
}

std::error_code
copy_directory(const std::shared_ptr<norns::io::task_info>& task_info,
               const bfs::path& src, const bfs::path& dst) {

    boost::system::error_code ec;
    auto it = bfs::recursive_directory_iterator(src, ec);
    const auto end = bfs::recursive_directory_iterator();
    std::size_t saved_sent_bytes = task_info->sent_bytes();

    if(ec) {
        return std::make_error_code(static_cast<std::errc>(ec.value()));
    }

    auto start = std::chrono::steady_clock::now();

    for(; it != end; ++it) {
        const auto dst_path = dst / bfs::relative(*it, src);

        if(bfs::is_directory(*it)) {
            if(!bfs::exists(dst_path)) {

                bfs::create_directory(dst_path, ec);

                if(ec) {
                    return std::make_error_code(static_cast<std::errc>(ec.value()));
                }
            }
            continue;
        }

        if(auto err = ::copy_file(task_info, *it, dst_path)) {
            return err;
        }
    }

    double usecs = std::chrono::duration<double, std::micro>(
            std::chrono::steady_clock::now() - start).count();

    std::size_t dir_bytes = task_info->sent_bytes() - saved_sent_bytes;
    task_info->update_bandwidth(dir_bytes, usecs);

    return std::make_error_code(static_cast<std::errc>(0));
}


} // namespace

namespace norns {
namespace io {

local_path_to_local_path_transferor::local_path_to_local_path_transferor(
    const context& ctx) :
        m_ctx(ctx) {}

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
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<const data::resource>& src,  
        const std::shared_ptr<const data::resource>& dst) const {

    (void) auth;

    const auto& d_src = 
        reinterpret_cast<const data::local_path_resource&>(*src);
    const auto& d_dst = 
        reinterpret_cast<const data::local_path_resource&>(*dst);

    LOGGER_DEBUG("[{}] transfer: {} -> {}", task_info->id(),
            d_src.canonical_path(), d_dst.canonical_path());

    if(bfs::is_directory(d_src.canonical_path())) {
        return ::copy_directory(task_info, d_src.canonical_path(), 
                                d_dst.canonical_path());
    }

    return ::copy_file(task_info, d_src.canonical_path(), 
                       d_dst.canonical_path());
}

std::error_code 
local_path_to_local_path_transferor::accept_transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<const data::resource>& src,  
        const std::shared_ptr<const data::resource>& dst) const {

    (void) auth;
    (void) task_info;
    (void) src;
    (void) dst;

    LOGGER_ERROR("This function should never be called for this transfer type");
    return std::make_error_code(static_cast<std::errc>(0));
}

std::string 
local_path_to_local_path_transferor::to_string() const {
    return "transferor[local_path => local_path]";
}


} // namespace io
} // namespace norns
