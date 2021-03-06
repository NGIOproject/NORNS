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

#include <sys/uio.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "utils.hpp"
#include "logger.hpp"
#include "resources.hpp"
#include "auth.hpp"
#include "io/task-info.hpp"
#include "memory-to-local-path.hpp"
#include "auth/process-credentials.hpp"

namespace {

std::error_code
copy_memory_region(const std::shared_ptr<norns::io::task_info>& task_info, 
                   pid_t pid, void* src_addr, size_t size, const bfs::path& dst) {

    int out_fd = -1;
    void* dst_addr = NULL;
    ssize_t nbytes = -1;
    int rv = 0;
    struct iovec local_region, remote_region;

    if(bfs::is_directory(dst)) {
        return std::make_error_code(static_cast<std::errc>(EISDIR));
    }

    auto start = std::chrono::steady_clock::now();

    // create and preallocate output file
    out_fd = ::open(dst.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if(out_fd == -1) {
        LOGGER_ERROR("open() error");
        return std::make_error_code(static_cast<std::errc>(errno));
    }

    // preallocate output file
#ifdef HAVE_FALLOCATE
    if(::fallocate(out_fd, 0, 0, size) == -1) {
        if(errno != EOPNOTSUPP) {
            LOGGER_ERROR("fallocate() error");
            rv = errno;
            goto cleanup_on_error;
        }
#endif // HAVE_FALLOCATE

        // filesystem doesn't support fallocate(), fallback to truncate()
        if(::ftruncate(out_fd, size) != 0) {
            LOGGER_ERROR("ftruncate() error on {}", dst);
            rv = errno;
            goto cleanup_on_error;
        }

#ifdef HAVE_FALLOCATE
    }
#endif // HAVE_FALLOCATE

    dst_addr = ::mmap(NULL, size, PROT_WRITE, MAP_SHARED, out_fd, 0);

    if(dst_addr == MAP_FAILED) {
        LOGGER_ERROR("mmap() error");
        rv = errno;
        goto cleanup_on_error;
    }

    local_region.iov_base = dst_addr;
    remote_region.iov_base = src_addr;
    local_region.iov_len = remote_region.iov_len = size;

    nbytes = ::process_vm_readv(pid, &local_region, 1, &remote_region, 1, 0);

    if(nbytes == -1) {
        LOGGER_ERROR("process_vm_readv() error");
        rv = errno;
        goto cleanup_on_error;
    }

    // according to the documentation, partial reads should only happen
    // at the granularity of iovec elements. Given that we only have one
    // element in our 'src' iovec, we should never hit this, but just in case
    // we return an EIO
    if(static_cast<size_t>(nbytes) < size) {
        LOGGER_ERROR("process_vm_readv() received fewer data than expected");
        rv = EIO;
        goto cleanup_on_error;
    }

    // success: set rv to 0 and fall through
    rv = 0;

cleanup_on_error:
    if(dst_addr != MAP_FAILED) {
        munmap(dst_addr, size);
    }

    if(out_fd != -1) {
retry_close:
        if(close(out_fd) == -1) {
            if(errno == EINTR) {
                goto retry_close;
            }
            LOGGER_ERROR("close() error");
            rv = errno;
        }
    }

    double usecs = std::chrono::duration<double, std::micro>(
            std::chrono::steady_clock::now() - start).count();

    if(rv == 0) {
        task_info->update_bandwidth(size, usecs);
    }

    return std::make_error_code(static_cast<std::errc>(rv));
}

}

namespace norns {
namespace io {

memory_region_to_local_path_transferor::
    memory_region_to_local_path_transferor(const context &ctx) :
        m_ctx(ctx) { }

bool 
memory_region_to_local_path_transferor::validate(
        const std::shared_ptr<data::resource_info>& src_info,
        const std::shared_ptr<data::resource_info>& dst_info) const {

    const auto& d_src = reinterpret_cast<const data::memory_region_info&>(*src_info);
    const auto& d_dst = reinterpret_cast<const data::local_path_info&>(*dst_info);

    // region length cannot be zero
    if(d_src.size() == 0) {
        return false;
    }

    // we don't allow destination paths that look like directories
    if(d_dst.datapath().back() == '/') {
        return false;
    }

    return true;
}

std::error_code 
memory_region_to_local_path_transferor::transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<const data::resource>& src,  
        const std::shared_ptr<const data::resource>& dst) const {

    (void) task_info;

    const auto& d_src = reinterpret_cast<const data::memory_region_resource&>(*src);
    const auto& d_dst = reinterpret_cast<const data::local_path_resource&>(*dst);

    LOGGER_DEBUG("[{}] transfer: [{} {}+{}] -> {}", task_info->id(), 
                 auth.pid(), utils::n2hexstr(d_src.address()), d_src.size(), 
                 d_dst.canonical_path());

    return ::copy_memory_region(task_info, auth.pid(), 
                                reinterpret_cast<void*>(d_src.address()), 
                                d_src.size(), d_dst.canonical_path());
}

std::error_code 
memory_region_to_local_path_transferor::accept_transfer(
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
memory_region_to_local_path_transferor::to_string() const {
    return "transferor[memory_region => local_path]";
}

} // namespace io
} // namespace norns
