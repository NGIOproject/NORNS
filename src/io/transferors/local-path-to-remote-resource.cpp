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
#include <stdlib.h>
#include "config.h"

#include "utils.hpp"
#include "logger.hpp"
#include "resources.hpp"
#include "auth.hpp"
#include "io/task-info.hpp"
#include "io/task-stats.hpp"
#include "hermes.hpp"
#include "rpcs.hpp"
#include "local-path-to-remote-resource.hpp"

namespace {

struct archive_entry {
    bool m_is_directory;
    bfs::path m_realpath;
    bfs::path m_archive_path;
};

bfs::path
pack_archive(const std::string& name_pattern,
             const bfs::path& parent_path,
             const std::vector<archive_entry>& entries,
             std::error_code& ec) {

    using norns::utils::tar;

    bfs::path ar_path = parent_path / bfs::unique_path(name_pattern);

    tar ar(ar_path, tar::create, ec);

    if(ec) {
        LOGGER_ERROR("Failed to create archive: {}", ec.message());
        return {};
    }

    LOGGER_INFO("Archive created in {}", ar.path());

    for(auto&& e : entries) {
        e.m_is_directory ?
            ar.add_directory(e.m_realpath, e.m_archive_path, ec) :
            ar.add_file(e.m_realpath, e.m_archive_path, ec);

        if(ec) {
            LOGGER_ERROR("Failed to add entry to archive: {}", ec.message());
            return {};
        }
    }

    return ar.path();
}

std::error_code
unpack_archive(const bfs::path& archive_path,
               const bfs::path& parent_path) {

    using norns::utils::tar;
    std::error_code ec;

    boost::system::error_code bec;
    tar ar(archive_path, tar::open, ec);

    if(ec) {
        LOGGER_ERROR("Failed to open archive {}: {}", 
                     archive_path, ec.message());
        return ec;
    }

    ar.extract(parent_path, ec);

    if(ec) {
        LOGGER_ERROR("Failed to extract archive {} into {}: {}",
                     ar.path(), parent_path, ec.message());
        return ec;
    }

    LOGGER_DEBUG("Archive {} extracted into {}, removing archive", 
                 ar.path(), parent_path);

    bfs::remove(ar.path(), bec);

    if(bec) {
        LOGGER_ERROR("Failed to remove archive {}: {}", 
                        ar.path(), bec.message());
        ec.assign(bec.value(), std::generic_category());
        return ec;
    }

    return ec;
}

} // anonymous namespace

namespace norns {
namespace io {

local_path_to_remote_resource_transferor::local_path_to_remote_resource_transferor(
        std::shared_ptr<hermes::async_engine> network_service) :
    m_network_service(network_service) { }

bool 
local_path_to_remote_resource_transferor::validate(
        const std::shared_ptr<data::resource_info>& src_info,
        const std::shared_ptr<data::resource_info>& dst_info) const {

    (void) src_info;
    (void) dst_info;

    LOGGER_WARN("Validation not implemented");

    return true;
}

std::error_code 
local_path_to_remote_resource_transferor::transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<const data::resource>& src,  
        const std::shared_ptr<const data::resource>& dst) const {

    using utils::tar;

    (void) auth;

    std::error_code ec;
    const auto& d_src = 
        reinterpret_cast<const data::local_path_resource&>(*src);
    const auto& d_dst = 
        reinterpret_cast<const data::remote_resource&>(*dst);
    auto tempfile = std::make_shared<utils::temporary_file>();

    bfs::path input_path = 
        !d_src.is_collection() ? 
        d_src.canonical_path() : 
        [&]() -> bfs::path {

            LOGGER_DEBUG("[{}] Creating temporary archive from local directory", 
                         task_info->id());

            const bfs::path ar_path =
                ::pack_archive("norns-archive-%%%%-%%%%-%%%%.tar",
                               "/tmp",
                               {{true, d_src.canonical_path(), d_dst.name()}},
                               ec);

            if(ec) {
                LOGGER_ERROR("Failed to create temporary archive: {}", 
                             ec.message());
                return {};
            }

            tempfile->manage(ar_path, ec);

            if(ec) {
                LOGGER_ERROR("Failed to create temporary archive: {}", 
                             ec.message());
                return {};
            }

            return tempfile->path();
        }(); // <<== XXX (IILE)


    LOGGER_DEBUG("[{}] start_transfer: {} -> {}", 
                 task_info->id(), d_src.canonical_path(), d_dst.to_string());

    hermes::endpoint endp = m_network_service->lookup(d_dst.address());

    try {
        hermes::mapped_buffer input_buffer(input_path.string(),
                                           hermes::access_mode::read_only,
                                           &ec);

        if(ec) {
            LOGGER_ERROR("Failed mapping input data: {}", ec.value());
            return ec;
        }

        std::vector<hermes::mutable_buffer> bufvec{
            hermes::mutable_buffer{input_buffer.data(), input_buffer.size()}
        };

        auto local_buffers = 
            m_network_service->expose(bufvec, hermes::access_mode::read_only);

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

        if(static_cast<task_status>(resp.at(0).status()) ==
            task_status::finished_with_error) {
            // XXX error interface should be improved
            return std::make_error_code(
                static_cast<std::errc>(resp.at(0).sys_errnum()));
        }

        task_info->record_transfer(input_buffer.size(), 
                                   resp.at(0).elapsed_time());

        LOGGER_DEBUG("Remote pull request completed with output "
                     "{{status: {}, task_error: {}, sys_errnum: {}}} "
                     "({} bytes, {} usecs)",
                    resp.at(0).status(), resp.at(0).task_error(), 
                    resp.at(0).sys_errnum(), input_buffer.size(), 
                    resp.at(0).elapsed_time());

        return ec;
    }
    catch(const std::exception& ex) {
        LOGGER_ERROR(ex.what());
        return std::make_error_code(static_cast<std::errc>(-1));
    }
}


std::error_code 
local_path_to_remote_resource_transferor::accept_transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<const data::resource>& src,  
        const std::shared_ptr<const data::resource>& dst) const {

    (void) auth;

//    LOGGER_CRITICAL("accept_remote_request: {}",
//                    std::chrono::duration_cast<std::chrono::nanoseconds>(
//                        std::chrono::steady_clock::now().time_since_epoch())
//                        .count());

    std::error_code ec;
    const auto& d_src = 
        reinterpret_cast<const data::remote_resource&>(*src);
    const auto& d_dst = 
        reinterpret_cast<const data::local_path_resource&>(*dst);

    // retrieve task context
    const auto ctx = boost::any_cast<
        std::shared_ptr<
            hermes::request<rpc::push_resource>>>(task_info->context());
    auto req = std::move(*ctx);

    LOGGER_DEBUG("[{}] accept_push: {} -> {}", task_info->id(),
            d_src.to_string(), d_dst.canonical_path());

    hermes::exposed_memory remote_buffers = d_src.buffers();

    LOGGER_DEBUG("remote_buffers{{count={}, total_size={}}}",
                 remote_buffers.count(),
                 remote_buffers.size());

    assert(remote_buffers.count() == 1);

    bool is_collection = d_src.is_collection();

    // TODO this should probably go into validate(), but we need to change
    // its interface to accept resources rather than resource_infos
    // to be able to determine whether d_dst is a directory
    if(d_src.name().empty() && bfs::is_directory(d_dst.canonical_path())) {
        LOGGER_ERROR("Failed to transfer unnamed resource to directory "
                     "(target should be a named file)");
        ec.assign(EISDIR, std::generic_category());
        *ctx = std::move(req); // restore ctx
        return ec;
    }

    auto tempfile = 
        std::make_shared<utils::temporary_file>(
            /* output_path */
            std::string{is_collection ? 
                "norns-archive-%%%%-%%%%-%%%%.tar" :
                d_dst.name()},
            /* parent_path */
            bfs::path{is_collection ?
                "/tmp" :
                d_dst.parent()->mount()},
            remote_buffers.size(),
            ec);

    if(ec) {
        LOGGER_ERROR("Failed to create temporary file: {}", ec.message());
        *ctx = std::move(req); // restore ctx
        return ec;
    }

    LOGGER_DEBUG("created local resource: {}", tempfile->path());

    auto output_buffer = 
        std::make_shared<hermes::mapped_buffer>(
                tempfile->path().string(),
                hermes::access_mode::write_only,
                &ec);

    if(ec) {
        LOGGER_ERROR("Failed mmapping output buffer: {}", ec.value());
        *ctx = std::move(req); // restore ctx
        return ec;
    }

    // let's prepare some local buffers
    std::vector<hermes::mutable_buffer> bufseq{
        hermes::mutable_buffer{output_buffer->data(), output_buffer->size()}
    };

    hermes::exposed_memory local_buffers =
        m_network_service->expose(bufseq, hermes::access_mode::write_only);

    LOGGER_DEBUG("pulling remote data into {}", tempfile->path());

    auto start = std::chrono::steady_clock::now();

    // N.B. IMPORTANT: we NEED to capture output_buffer by value here so that
    // the mapped_buffer doesn't get released before completion_callback()
    // is called.
    const auto completion_callback = 
        [this, is_collection, tempfile, d_dst, output_buffer, start](
            hermes::request<rpc::push_resource>&& req) {

//        LOGGER_CRITICAL("completion_callback invoked: {}",
//                        std::chrono::duration_cast<std::chrono::nanoseconds>(
//                            std::chrono::steady_clock::now().time_since_epoch())
//                            .count());

        uint32_t usecs = 
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start).count();

        //TODO: hermes offers no way to check for an error yet
        LOGGER_DEBUG("Pull completed ({} usecs)", usecs);

        // default response (success)
        rpc::push_resource::output out = {
                static_cast<uint32_t>(task_status::finished),
                static_cast<uint32_t>(urd_error::success),
                0,
                usecs};

        if(is_collection) {
            std::error_code ec = 
                ::unpack_archive(tempfile->path(), d_dst.parent()->mount());

            if(ec) {
                out = rpc::push_resource::output{
                        static_cast<uint32_t>(
                            task_status::finished_with_error),
                        static_cast<uint32_t>(urd_error::system_error),
                        static_cast<uint32_t>(ec.value()),
                        0};

                goto respond;
            }

            LOGGER_DEBUG("Archive {} extracted into {}", 
                         tempfile->path(), d_dst.parent()->mount());

            goto respond;
        }

        // prevent output file from being removed by tempfile's destructor
        (void) tempfile->release();

respond:
        if(req.requires_response()) {
            m_network_service->respond<rpc::push_resource>(
                    std::move(req), out);
        }
    };

//    LOGGER_CRITICAL("async_pull posted: {}",
//                    std::chrono::duration_cast<std::chrono::nanoseconds>(
//                        std::chrono::steady_clock::now().time_since_epoch())
//                        .count());

    m_network_service->async_pull(remote_buffers,
                                  local_buffers,
                                  std::move(req),
                                  completion_callback);

    return ec;
}

std::string 
local_path_to_remote_resource_transferor::to_string() const {
    return "transferor[local_path => remote_resource]";
}

} // namespace io
} // namespace norns
