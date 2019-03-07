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

#include "utils.hpp"
#include "logger.hpp"
#include "resources.hpp"
#include "auth.hpp"
#include "io/task-info.hpp"
#include "io/task-stats.hpp"
#include "memory-to-remote-resource.hpp"

namespace {

std::error_code
copy_from_process(pid_t pid, 
                  void* input_address, 
                  void* output_address, 
                  size_t size) {

    std::error_code ec;

    struct iovec local_region, remote_region;
    local_region.iov_base = output_address;
    remote_region.iov_base = input_address;
    local_region.iov_len = remote_region.iov_len = size;

    ssize_t nbytes =
        ::process_vm_readv(pid, &local_region, 1, &remote_region, 1, 0);

    if(nbytes == -1) {
        LOGGER_ERROR("process_vm_readv() error");
        ec.assign(errno, std::generic_category());
        return ec;
    }

    // according to the documentation, partial reads should only happen
    // at the granularity of iovec elements. Given that we only have one
    // element in our 'src' iovec, we should never hit this, but just in case
    // we return an EIO
    if(static_cast<size_t>(nbytes) < size) {
        LOGGER_ERROR("process_vm_readv() received fewer data than expected");
        ec.assign(EIO, std::generic_category());
        return ec;
    }

    if(::msync(output_address, size, MS_SYNC) != 0) {
        LOGGER_ERROR("Failed to sync mapped buffer to file");
        ec.assign(errno, std::generic_category());
        return ec;
    }

    return ec;
}

} // anonymous namespace

namespace norns {
namespace io {

memory_region_to_remote_resource_transferor::
    memory_region_to_remote_resource_transferor(
        std::shared_ptr<hermes::async_engine> network_service) :
    m_network_service(network_service) {}

bool 
memory_region_to_remote_resource_transferor::validate(
        const std::shared_ptr<data::resource_info>& src_info,
        const std::shared_ptr<data::resource_info>& dst_info) const {

    const auto& d_src = 
        reinterpret_cast<const data::memory_region_info&>(*src_info);
    const auto& d_dst = 
        reinterpret_cast<const data::remote_resource_info&>(*dst_info);

    // region length cannot be zero
    if(d_src.size() == 0) {
        return false;
    }

    // we don't allow destination paths that look like directories
    if(d_dst.name().back() == '/') {
        return false;
    }

    return true;
}

std::error_code 
memory_region_to_remote_resource_transferor::transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<const data::resource>& src,  
        const std::shared_ptr<const data::resource>& dst) const {

    (void) auth;
    (void) task_info;

    std::error_code ec;
    const auto& d_src = 
        reinterpret_cast<const data::memory_region_resource&>(*src);
    const auto& d_dst = 
        reinterpret_cast<const data::remote_resource&>(*dst);

    LOGGER_DEBUG("[{}] transfer: [{} {}+{}] -> {}", task_info->id(), 
                 auth.pid(), utils::n2hexstr(d_src.address()), d_src.size(), 
                 d_dst.to_string());

    // copy the user buffer to a temporary file local file using 
    // ::process_vm_readv() so that we can safely send it to remote peers
    auto tempfile =
        std::make_shared<utils::temporary_file>(
            std::string{"norns-tempfile-%%%%-%%%%-%%%%"},
            bfs::path{"/tmp"},
            d_src.size(),
            ec);

    if(ec) {
        LOGGER_ERROR("Failed to create temporary file: {}", ec.message());
        return ec;
    }

    LOGGER_DEBUG("created temporary file: {}", tempfile->path());

    auto output_buffer = 
        std::make_shared<hermes::mapped_buffer>(
                tempfile->path().string(),
                hermes::access_mode::write_only,
                &ec);

    if(ec) {
        LOGGER_ERROR("Failed mapping output buffer: {}", ec.value());
        return ec;
    }

    if((ec = ::copy_from_process(auth.pid(),
                                reinterpret_cast<void *>(d_src.address()),
                                output_buffer->data(), d_src.size()))) {
        LOGGER_ERROR("Failed to copy data from process memory: {}", 
                     ec.message());
        return ec;
    }

    // TODO: release() + remap()
    output_buffer->protect(hermes::access_mode::read_only, &ec);

    if(ec) {
        LOGGER_ERROR("Failed changing protections from output buffer: {}", 
                     ec.value());
        return ec;
    }

    try {

        hermes::endpoint endp = m_network_service->lookup(d_dst.address());

        std::vector<hermes::mutable_buffer> bufvec{
            hermes::mutable_buffer{output_buffer->data(), output_buffer->size()}
        };

        auto local_buffers = 
            m_network_service->expose(bufvec, hermes::access_mode::read_only);

//        LOGGER_CRITICAL("push_resource RPC posted: {}",
//                        std::chrono::duration_cast<std::chrono::nanoseconds>(
//                            std::chrono::steady_clock::now().time_since_epoch())
//                            .count());

        auto resp = 
            m_network_service->post<rpc::push_resource>(
                endp, 
                rpc::push_resource::input{
                    m_network_service->self_address(),
                    d_src.parent()->nsid(),
                    d_dst.parent()->nsid(), 
                    // XXX this resource_type should not be needed, but we
                    // XXX cannot (easily) find it out right now in the server, 
                    // XXX for now we propagate it, but we should implement
                    // XXX a lookup()/stat() function in backends to retrieve
                    // XXX this information locally from the resource id
                    static_cast<uint32_t>(
                        data::resource_type::local_posix_path), 
                    d_src.is_collection(),
                    d_src.name(),
                    d_dst.name(),
                    local_buffers
                }).get();

//        LOGGER_CRITICAL("push_resource response retrieved: {}",
//                        std::chrono::duration_cast<std::chrono::nanoseconds>(
//                            std::chrono::steady_clock::now().time_since_epoch())
//                            .count());

        if(static_cast<task_status>(resp.at(0).status()) ==
            task_status::finished_with_error) {
            // XXX error interface should be improved
            return std::make_error_code(
                static_cast<std::errc>(resp.at(0).sys_errnum()));
        }

        task_info->record_transfer(output_buffer->size(), 
                                   resp.at(0).elapsed_time());

        LOGGER_DEBUG("Remote pull request completed with output "
                     "{{status: {}, task_error: {}, sys_errnum: {}}} "
                     "({} bytes, {} usecs)",
                    resp.at(0).status(), resp.at(0).task_error(), 
                    resp.at(0).sys_errnum(), output_buffer->size(), 
                    resp.at(0).elapsed_time());

        return ec;
    }
    catch(const std::exception& ex) {
        LOGGER_ERROR(ex.what());
        return std::make_error_code(static_cast<std::errc>(-1));
    }

    return ec;
}

std::error_code 
memory_region_to_remote_resource_transferor::accept_transfer(
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
memory_region_to_remote_resource_transferor::to_string() const {
    return "transferor[memory_region => remote_resource]";
}

} // namespace io
} // namespace norns
