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

    if(::fallocate(out_fd, 0, 0, size) == -1) {
        // filesystem doesn't support fallocate(), fallback to truncate()
        if(errno == EOPNOTSUPP) {
            if(::ftruncate(out_fd, size) != 0) {
                LOGGER_ERROR("ftruncate() error on {}", dst);
                rv = errno;
                goto cleanup_on_error;
            }
        }
        LOGGER_ERROR("fallocate() error");
        rv = errno;
        goto cleanup_on_error;
    }

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
memory_region_to_local_path_transferor::transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<data::resource_info>& src_rinfo,
        const std::shared_ptr<data::resource_info>& dst_rinfo) const {

    (void) task_info;
    std::error_code ec;
    const auto src_backend = task_info->src_backend();
    const auto dst_backend = task_info->dst_backend();

    auto src = src_backend->get_resource(src_rinfo, ec);

    if(ec) {
        LOGGER_ERROR("[{}] Could not access input resource '{}': {}", 
                     task_info->id(),
                     src_rinfo->to_string(),
                     ec.message());
        return ec;
    }

    auto dst = dst_backend->new_resource(dst_rinfo, src->is_collection(), ec);

    if(ec) {
        LOGGER_ERROR("[{}] Could not create output resource '{}': {}", 
                     task_info->id(),
                     src_rinfo->to_string(),
                     ec.message());
        return ec;
    }

    const auto& d_src = reinterpret_cast<const data::memory_region_resource&>(*src);
    const auto& d_dst = reinterpret_cast<const data::local_path_resource&>(*dst);

    LOGGER_DEBUG("[{}] transfer: [{} {}+{}] -> {}", 
                 task_info->id(), 
                 auth.pid(), 
                 utils::n2hexstr(d_src.address()), 
                 d_src.size(), 
                 d_dst.canonical_path());

    return ::copy_memory_region(task_info, auth.pid(), 
                                reinterpret_cast<void*>(d_src.address()), 
                                d_src.size(), d_dst.canonical_path());
}

std::string 
memory_region_to_local_path_transferor::to_string() const {
    return "transferor[memory_region => local_path]";
}

} // namespace io
} // namespace norns
