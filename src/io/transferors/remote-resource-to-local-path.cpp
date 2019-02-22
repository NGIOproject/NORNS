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
        std::shared_ptr<hermes::async_engine> remote_endpoint) :
    m_remote_endpoint(remote_endpoint) { }

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

    using utils::tar;

    const auto& d_src = 
        reinterpret_cast<const data::remote_resource&>(*src);
    const auto& d_dst = 
        reinterpret_cast<const data::local_path_resource&>(*dst);

    const auto ctx = boost::any_cast<
        std::shared_ptr<
            hermes::request<rpc::remote_transfer>>>(task_info->context());
    auto req = std::move(*ctx);

    LOGGER_DEBUG("[{}] accept_transfer: {} -> {}", task_info->id(),
            d_src.to_string(), d_dst.canonical_path());

    LOGGER_WARN("Invoking rpc::remote_transfer({}) on {}", d_dst.name(), "xxx");

    hermes::exposed_memory remote_buffers = d_src.buffers();

    LOGGER_DEBUG("remote_buffers{{count={}, total_size={}}}",
                 remote_buffers.count(),
                 remote_buffers.size());

    assert(remote_buffers.count() == 1);

    bool is_collection = d_src.is_collection();

    bfs::path output_path = 
        is_collection ? "/tmp/test.tar" : d_dst.canonical_path();

    LOGGER_DEBUG("creating local resource: {}", output_path);

    std::error_code ec;
    std::shared_ptr<hermes::mapped_buffer> output_data;

    std::tie(ec, output_data) = 
        ::create_file(output_path, remote_buffers.size());

    if(ec) {
        *ctx = std::move(req);
        return ec;
    }

    // let's prepare some local buffers
    
    std::vector<hermes::mutable_buffer> bufseq{
        hermes::mutable_buffer{output_data->data(), output_data->size()}
    };

    hermes::exposed_memory local_buffers =
        m_remote_endpoint->expose(bufseq, hermes::access_mode::write_only);

    LOGGER_DEBUG("pulling remote data into {}", output_path);

    auto start = std::chrono::steady_clock::now();

    // N.B. IMPORTANT: we NEED to capture output_data by value here so that
    // the mapped_buffer doesn't get released before completion_callback()
    // is called.
    const auto completion_callback = 
        [this, is_collection, output_path, d_dst, output_data, start](
            hermes::request<rpc::remote_transfer>&& req) {

        uint32_t usecs = 
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start).count();

        // default response
        rpc::remote_transfer::output out(
                static_cast<uint32_t>(task_status::finished),
                static_cast<uint32_t>(urd_error::success),
                0,
                usecs);

        //TODO: hermes offers no way to check for an error yet
        LOGGER_DEBUG("Transfer completed ({} usecs)", usecs);

        if(is_collection) {
            std::error_code ec;
            boost::system::error_code bec;
            tar ar(output_path, tar::open, ec);

            if(ec) {
                LOGGER_ERROR("Failed to open archive {}: {}", 
                             output_path, logger::errno_message(ec.value()));

                out = std::move(rpc::remote_transfer::output{
                    static_cast<uint32_t>(task_status::finished_with_error),
                    static_cast<uint32_t>(urd_error::system_error),
                    static_cast<uint32_t>(ec.value()),
                    0
                });
                goto respond;
            }

            ar.extract(d_dst.parent()->mount(), ec);

            if(ec) {
                LOGGER_ERROR("Failed to extact archive into {}: {}",
                             ar.path(), d_dst.parent()->mount(), 
                             logger::errno_message(ec.value()));
                out = std::move(rpc::remote_transfer::output{
                    static_cast<uint32_t>(task_status::finished_with_error),
                    static_cast<uint32_t>(urd_error::system_error),
                    static_cast<uint32_t>(ec.value()),
                    0
                });
                goto respond;
            }

            LOGGER_DEBUG("Archive {} extracted into {}", 
                         ar.path(), d_dst.parent()->mount());

            bfs::remove(ar.path(), bec);

            if(bec) {
                LOGGER_ERROR("Failed to remove archive {}: {}", 
                             ar.path(), logger::errno_message(ec.value()));
                out = std::move(rpc::remote_transfer::output{
                    static_cast<uint32_t>(task_status::finished_with_error),
                    static_cast<uint32_t>(urd_error::system_error),
                    static_cast<uint32_t>(ec.value()),
                    0
                });
            }
        }

respond:
        if(req.requires_response()) {
            m_remote_endpoint->respond<rpc::remote_transfer>(
                    std::move(req), 
                    static_cast<uint32_t>(task_status::finished),
                    static_cast<uint32_t>(urd_error::success),
                    0,
                    usecs);
        }
    };

    m_remote_endpoint->async_pull(remote_buffers,
                                  local_buffers,
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
