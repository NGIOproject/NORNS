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
#include "hermes.hpp"
#include "rpcs.hpp"
#include "remote-resource-to-local-path.hpp"

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

remote_resource_to_local_path_transferor::remote_resource_to_local_path_transferor(
        std::shared_ptr<hermes::async_engine> network_endpoint) :
    m_network_endpoint(network_endpoint) { }

bool
remote_resource_to_local_path_transferor::validate(
        const std::shared_ptr<data::resource_info>& src_info,
        const std::shared_ptr<data::resource_info>& dst_info) const {

    (void) src_info;
    (void) dst_info;

    LOGGER_WARN("Validation not implemented");

    return true;
}

std::error_code 
remote_resource_to_local_path_transferor::transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<const data::resource>& src,  
        const std::shared_ptr<const data::resource>& dst) const {

    (void) auth;

    std::error_code ec;
    const auto& d_src = 
        reinterpret_cast<const data::remote_resource&>(*src);
    const auto& d_dst = 
        reinterpret_cast<const data::local_path_resource&>(*dst);

    LOGGER_DEBUG("[{}] request_transfer: {} -> {}", 
                 task_info->id(), d_src.to_string(), d_dst.canonical_path());

    hermes::endpoint endp = m_network_endpoint->lookup(d_src.address());

    auto resp = 
        m_network_endpoint->post<rpc::resource_stat>(
            endp, 
            rpc::resource_stat::input{
                m_network_endpoint->self_address(),
                d_src.parent()->nsid(),
                static_cast<uint32_t>(data::resource_type::local_posix_path), 
                d_src.name()
            }).get();

    LOGGER_DEBUG("remote_stat returned [task_error: {}, sys_errnum: {}, "
                 "is_collection: {}, packed_size: {}]", 
                 resp.at(0).task_error(),
                 resp.at(0).sys_errnum(),
                 resp.at(0).is_collection(),
                 resp.at(0).packed_size());

    if(static_cast<urd_error>(resp.at(0).task_error()) !=
            urd_error::success) {
        return std::make_error_code(
            static_cast<std::errc>(resp.at(0).sys_errnum()));
    }

    utils::temporary_file tempfile(
        /* output_path */
        {resp.at(0).is_collection() ?
            "norns-archive-%%%%-%%%%-%%%%.tar" :
            d_dst.name()},
        /* parent_dir */
        {resp.at(0).is_collection() ? 
            "/tmp" : 
            d_dst.parent()->mount()},
        resp.at(0).packed_size(), 
        ec);

    if(ec) {
        LOGGER_ERROR("Failed to create temporary file: {}", ec.message());
        return ec;
    }

    LOGGER_DEBUG("created local resource: {}", tempfile.path());

    auto output_buffer = 
        std::make_shared<hermes::mapped_buffer>(
                tempfile.path().string(),
                hermes::access_mode::write_only,
                &ec);

    if(ec) {
        LOGGER_ERROR("Failed mmapping output buffer: {}", ec.value());
        return ec;
    }

    // let's prepare some local buffers
    std::vector<hermes::mutable_buffer> bufseq{
        hermes::mutable_buffer{output_buffer->data(), output_buffer->size()}
    };

    hermes::exposed_memory local_buffers =
        m_network_endpoint->expose(bufseq, hermes::access_mode::write_only);

    auto resp2 = 
        m_network_endpoint->post<rpc::pull_resource>(
            endp,
            rpc::pull_resource::input{
                d_src.parent()->nsid(), 
                d_src.name(),
                // XXX this resource_type should not be needed, but we
                // XXX cannot (easily) find it out right now in the server, 
                // XXX for now we propagate it, but we should implement
                // XXX a lookup()/stat() function in backends to retrieve
                // XXX this information from the resource id
                static_cast<uint32_t>(
                    data::resource_type::local_posix_path),
                m_network_endpoint->self_address(), 
                d_dst.parent()->nsid(),
                d_dst.name(), 
                local_buffers
            }).get();

    LOGGER_DEBUG("Remote push request completed with output "
                 "{{status: {}, task_error: {}, sys_errnum: {}}} "
                 "({} bytes, {} usecs)",
                 resp2.at(0).status(), resp2.at(0).task_error(), 
                 resp2.at(0).sys_errnum(), output_buffer->size(), 
                 resp2.at(0).elapsed_time());

    if(static_cast<task_status>(resp2.at(0).status()) ==
        task_status::finished_with_error) {
        // XXX error interface should be improved
        return std::make_error_code(
            static_cast<std::errc>(resp2.at(0).sys_errnum()));
    }

    task_info->record_transfer(output_buffer->size(), 
                               resp2.at(0).elapsed_time());

    if(resp.at(0).is_collection()) {
        return ::unpack_archive(tempfile.path(), d_dst.parent()->mount());
    }

    // prevent output file from being removed by tempfile's destructor
    (void) tempfile.release();

    return ec;
}


std::error_code 
remote_resource_to_local_path_transferor::accept_transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<const data::resource>& src,  
        const std::shared_ptr<const data::resource>& dst) const {

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

    // retrieve task context
    const auto ctx = boost::any_cast<
        std::shared_ptr<
            hermes::request<rpc::pull_resource>>>(task_info->context());
    auto req = std::move(*ctx);

    LOGGER_DEBUG("[{}] accept_pull: {} -> {}", task_info->id(),
            d_src.canonical_path(), d_dst.to_string());

    // create local buffers from local input data
    auto input_buffer = 
        std::make_shared<hermes::mapped_buffer>(
                input_path.string(),
                hermes::access_mode::read_only,
                &ec);

    if(ec) {
        LOGGER_ERROR("Failed mapping input data: {}", ec.message());
        *ctx = std::move(req); // restore ctx
        return ec;
    }

    std::vector<hermes::mutable_buffer> bufvec{
        hermes::mutable_buffer{input_buffer->data(), input_buffer->size()}
    };

    auto local_buffers = 
        m_network_endpoint->expose(bufvec, hermes::access_mode::read_only);

    // retrieve remote buffers descriptor
    hermes::exposed_memory remote_buffers = d_dst.buffers();

    LOGGER_DEBUG("remote_buffers{{count={}, total_size={}}}",
                 remote_buffers.count(),
                 remote_buffers.size());

    auto start = std::chrono::steady_clock::now();

    // N.B. IMPORTANT: we NEED to capture 'input_buffer' by value here so that
    // the mapped_buffer doesn't get released before completion_callback()
    // is called.
    // We also capture 'tempfile' by value so that it gets automatically 
    // released (and the associated file erased) when the callback finishes
    // FIXME: with C++14 we could simply std::move both into the capture rather
    // than using shared_ptrs :/
    const auto completion_callback =
        [this, tempfile, input_buffer, start](
                hermes::request<rpc::pull_resource>&& req) { 

        uint32_t usecs = 
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start).count();

        // default response
        rpc::pull_resource::output out(
                static_cast<uint32_t>(task_status::finished),
                static_cast<uint32_t>(urd_error::success),
                0,
                usecs);

        //TODO: hermes offers no way to check for an error yet
        LOGGER_DEBUG("Push completed");

        if(req.requires_response()) {
            m_network_endpoint->respond<rpc::pull_resource>(
                    std::move(req), 
                    out);
        }
    };

    m_network_endpoint->async_push(local_buffers, 
                                   remote_buffers, 
                                   std::move(req),
                                   completion_callback);

    return ec;
}

std::string 
remote_resource_to_local_path_transferor::to_string() const {
    return "transferor[remote_resource => local_path]";
}

} // namespace io
} // namespace norns
