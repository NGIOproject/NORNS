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

std::tuple<std::error_code, std::shared_ptr<hermes::mapped_buffer>>
create_file(const bfs::path& filename,
            std::size_t size) {

    std::error_code ec;

    int out_fd = 
        ::open(filename.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

    if(out_fd == -1) {
        ec = std::make_error_code(static_cast<std::errc>(errno));
        return std::make_tuple(ec, nullptr);
    }

    // preallocate output file
    if(::fallocate(out_fd, 0, 0, size) == -1) {
        // filesystem doesn't support fallocate(), fallback to truncate()
        if(errno == EOPNOTSUPP) {
            if(::ftruncate(out_fd, size) != 0) {
                ec = std::make_error_code(static_cast<std::errc>(errno));
                return std::make_tuple(ec, nullptr);
            }
        }
        ec = std::make_error_code(static_cast<std::errc>(errno));
        return std::make_tuple(ec, nullptr);
    }

retry_close:
    if(close(out_fd) == -1) {
        if(errno == EINTR) {
            goto retry_close;
        }

        ec = std::make_error_code(static_cast<std::errc>(errno));
        return std::make_tuple(ec, nullptr);
    }

    auto output_data = 
        std::make_shared<hermes::mapped_buffer>(
            filename.string(), 
            hermes::access_mode::write_only,
            &ec);

    if(ec) {
        LOGGER_ERROR("Failed mapping output data: {}", ec.value());
        return std::make_tuple(ec, output_data);
    }

    return std::make_tuple(ec, output_data);
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
    (void) src;
    (void) dst;
    (void) task_info;

    const auto& d_src = 
        reinterpret_cast<const data::remote_resource&>(*src);
    const auto& d_dst = 
        reinterpret_cast<const data::local_path_resource&>(*dst);

    LOGGER_DEBUG("[{}] request_transfer: {} -> {}", 
                 task_info->id(), d_src.to_string(), d_dst.canonical_path());

    hermes::endpoint endp = m_network_endpoint->lookup(d_src.address());

    rpc::resource_stat::input args(
            m_network_endpoint->self_address(),
            d_src.parent()->nsid(),
            static_cast<uint32_t>(data::resource_type::local_posix_path), 
            d_src.name());

    auto resp = 
        m_network_endpoint->post<rpc::resource_stat>(endp, args).get();

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

    bfs::path output_path = 
        resp.at(0).is_collection() ? "/tmp/test.tar" : d_dst.canonical_path();

    LOGGER_DEBUG("creating local resource: {}", output_path);

    std::error_code ec;
    std::shared_ptr<hermes::mapped_buffer> output_buffer;

    std::tie(ec, output_buffer) = 
        ::create_file(output_path, resp.at(0).packed_size());

    // let's prepare some local buffers
    std::vector<hermes::mutable_buffer> bufseq{
        hermes::mutable_buffer{output_buffer->data(), output_buffer->size()}
    };

    hermes::exposed_memory local_buffers =
        m_network_endpoint->expose(bufseq, hermes::access_mode::write_only);

    rpc::pull_resource::input args2(
            d_src.parent()->nsid(),
            d_src.name(),
            // XXX this resource_type should not be needed, but we cannot
            // XXX (easily) find it out right now in the server, for now we
            // XXX propagate it, but we should implement a lookup()/stat()
            // XXX function in backends to retrieve this information from the
            // XXX resource id
            static_cast<uint32_t>(data::resource_type::local_posix_path), 
            m_network_endpoint->self_address(),
            d_dst.parent()->nsid(), 
            d_dst.name(),
            local_buffers);

    auto resp2 = 
        m_network_endpoint->post<rpc::pull_resource>(endp, args2).get();

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

    if(resp.at(0).is_collection()) {
        using utils::tar;

        std::error_code ec;
        boost::system::error_code bec;
        tar ar(output_path, tar::open, ec);

        if(ec) {
            LOGGER_ERROR("Failed to open archive {}: {}", 
                         output_path, logger::errno_message(ec.value()));

            return std::make_error_code(static_cast<std::errc>(ec.value()));
        }

        ar.extract(d_dst.parent()->mount(), ec);

        if(ec) {
            LOGGER_ERROR("Failed to extact archive into {}: {}",
                         ar.path(), d_dst.parent()->mount(), ec.message());
            return std::make_error_code(static_cast<std::errc>(ec.value()));
        }

        LOGGER_DEBUG("Archive {} extracted into {}", 
                     ar.path(), d_dst.parent()->mount());

        bfs::remove(ar.path(), bec);

        if(bec) {
            LOGGER_ERROR("Failed to remove archive {}: {}", 
                         ar.path(), ec.message());
            return std::make_error_code(static_cast<std::errc>(ec.value()));
        }
    }

    return std::make_error_code(static_cast<std::errc>(0));
}


std::error_code 
remote_resource_to_local_path_transferor::accept_transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<const data::resource>& src,  
        const std::shared_ptr<const data::resource>& dst) const {

    (void) auth;
    (void) task_info;
    (void) src;
    (void) dst;

    const auto& d_src = 
        reinterpret_cast<const data::local_path_resource&>(*src);
    const auto& d_dst = 
        reinterpret_cast<const data::remote_resource&>(*dst);
    std::error_code ec;

    std::string input_path = d_src.canonical_path().string();
    bool is_collection = d_src.is_collection();

    if(is_collection) {
        using utils::tar;

        LOGGER_DEBUG("[{}] Creating archive from local directory", 
                     task_info->id());

        bfs::path ar_path = 
            "/tmp" / bfs::unique_path("norns-archive-%%%%-%%%%-%%%%.tar");
        
        tar ar(ar_path, tar::create, ec);

        if(ec) {
            LOGGER_ERROR("Failed to create archive: {}", 
                         logger::errno_message(ec.value()));
            return ec;
        }

        LOGGER_INFO("Archive created in {}", ar.path());

        ar.add_directory(d_src.canonical_path(), 
                         d_dst.name(),
                         ec);

        if(ec) {
            LOGGER_ERROR("Failed to add directory to archive: {}", 
                         logger::errno_message(ec.value()));
            return ec;
        }

        input_path = ar.path().string();
    }

    // retrieve task context
    const auto ctx = boost::any_cast<
        std::shared_ptr<
            hermes::request<rpc::pull_resource>>>(task_info->context());
    auto req = std::move(*ctx);

    LOGGER_DEBUG("[{}] accept_pull: {} -> {}", task_info->id(),
            d_src.canonical_path(), d_dst.to_string());

    // create local buffers from local input data
    auto input_buffer = 
        std::make_shared<hermes::mapped_buffer>(input_path,
                                                hermes::access_mode::read_only,
                                                &ec);

    if(ec) {
        LOGGER_ERROR("Failed mapping input data: {}", ec.message());
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

    // N.B. IMPORTANT: we NEED to capture input_buffer by value here so that
    // the mapped_buffer doesn't get released before completion_callback()
    // is called.
    const auto completion_callback =
        [this, is_collection, input_path, input_buffer](
                hermes::request<rpc::pull_resource>&& req) { 

        // default response
        rpc::pull_resource::output out(
                static_cast<uint32_t>(task_status::finished),
                static_cast<uint32_t>(urd_error::success),
                0,
                0);

        //TODO: hermes offers no way to check for an error yet
        LOGGER_DEBUG("Push completed");

        if(is_collection) {
            boost::system::error_code bec;
            bfs::remove(input_path, bec);

            if(bec) {
                LOGGER_ERROR("Failed to remove archive {}: {}", 
                             input_path, bec.message());
                out = std::move(rpc::pull_resource::output{
                    static_cast<uint32_t>(task_status::finished_with_error),
                    static_cast<uint32_t>(urd_error::system_error),
                    static_cast<uint32_t>(bec.value()),
                    0
                });
                goto respond;
            }
        }

respond:
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

    return std::make_error_code(static_cast<std::errc>(0));
}

std::string 
remote_resource_to_local_path_transferor::to_string() const {
    return "transferor[remote_resource => local_path]";
}

} // namespace io
} // namespace norns
