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

    LOGGER_DEBUG("creating local resource: {}", d_dst.canonical_path());

    std::error_code ec;
    std::shared_ptr<hermes::mapped_buffer> output_data;

    std::tie(ec, output_data) = 
        ::create_file(d_dst.canonical_path(), remote_buffers.size());

    if(ec) {
        if(req.requires_response()) {
            m_remote_endpoint->respond<rpc::remote_transfer>(
                    std::move(req),
                    static_cast<uint32_t>(task_status::finished_with_error),
                    static_cast<uint32_t>(urd_error::system_error),
                    static_cast<uint32_t>(ec.value()));
        }
        return ec;
    }

    // let's prepare some local buffers
    
    std::vector<hermes::mutable_buffer> bufseq{
        hermes::mutable_buffer{output_data->data(), output_data->size()}
    };

    hermes::exposed_memory local_buffers =
        m_remote_endpoint->expose(bufseq, hermes::access_mode::write_only);

    LOGGER_DEBUG("pulling remote data into {}", d_dst.canonical_path());

    // N.B. IMPORTANT: we NEED to capture output_data by value here so that
    // the mapped_buffer doesn't get released before completion_callback()
    // is called.
    const auto completion_callback = [this, output_data](
            hermes::request<rpc::remote_transfer>&& req) {

        //TODO: hermes offers no way to check for an error yet
        LOGGER_DEBUG("pull succeeded");

        if(req.requires_response()) {
            m_remote_endpoint->respond<rpc::remote_transfer>(
                    std::move(req), 
                    static_cast<uint32_t>(task_status::finished),
                    static_cast<uint32_t>(urd_error::success),
                    0);
        }
    };

    m_remote_endpoint->async_pull(remote_buffers,
                                  local_buffers,
                                  std::move(req),
                                  completion_callback);

    return std::make_error_code(static_cast<std::errc>(0));
}

std::error_code 
remote_resource_to_local_path_transferor::transfer(
        const auth::credentials& auth, 
        const std::shared_ptr<task_info>& task_info,
        const std::shared_ptr<data::resource_info>& src_rinfo,  
        const std::shared_ptr<data::resource_info>& dst_rinfo) const {

    (void) auth;
    (void) task_info;
    (void) src_rinfo;
    (void) dst_rinfo;

#if 0
    (void) auth;
    const auto src_backend = task_info->src_backend();
    const auto dst_backend = task_info->dst_backend();

    std::error_code ec;
    auto src = src_backend->get_resource(src_rinfo, ec);

    if(ec) {
        LOGGER_ERROR("[{}] Could not access input resource '{}': {}", 
                     task_info->id(),
                     src_rinfo->to_string(),
                     ec.message());
        return ec;
    }

    const auto& d_src = 
        reinterpret_cast<const data::local_path_resource&>(*src);

    const auto& d_dst =
        reinterpret_cast<const data::remote_path_info&>(*dst_rinfo);

    LOGGER_DEBUG("[{}] transfer: {} -> {}://{}@{}", 
                 task_info->id(),
                 d_src.canonical_path(),
                 d_dst.nsid(),
                 d_dst.datapath(),
                 d_dst.hostname());

    LOGGER_WARN("Invoking rpc::remote_transfer({}) on {}", 
                dst_rinfo->to_string(), 
                "xxx");

    hermes::endpoint endp = m_remote_endpoint->lookup(d_dst.hostname());

    hermes::mapped_buffer b(d_src.canonical_path().string());

    std::vector<hermes::mutable_buffer> bufvec{
        hermes::mutable_buffer{b.data(), b.size()}
    };

    auto buffers = 
        m_remote_endpoint->expose(bufvec, hermes::access_mode::read_only);

    rpc::remote_transfer::input 
        args(d_dst.nsid(), d_dst.datapath(), buffers);

    auto rpc = 
        m_remote_endpoint->post<rpc::remote_transfer>(endp, args);
#endif

#if 0
    const auto& d_src = 
        reinterpret_cast<const data::local_path_resource&>(*src);
    const auto& d_dst = 
        reinterpret_cast<const data::remote_path_resource&>(*dst);

    LOGGER_DEBUG("[{}] transfer: {} -> {}", task_info->id(),
            d_src.canonical_path(), d_dst.to_string());

    LOGGER_WARN("Invoking rpc::remote_transfer({}) on {}", d_dst.name(), "xxx");

    hermes::endpoint endp = m_remote_endpoint->lookup("127.0.0.1:42000");

    char* data = new char[42];

    std::vector<hermes::mutable_buffer> bufvec{
        hermes::mutable_buffer{(void*) data, 42}
    };

    auto buffers = 
        m_remote_endpoint->expose(bufvec, hermes::access_mode::read_only);

    norns::rpc::remote_transfer::input args("/tmp/foo", buffers);

    auto rpc = 
        m_remote_endpoint->post<norns::rpc::remote_transfer>(endp, args);


#endif
    return std::make_error_code(static_cast<std::errc>(0));
}

std::string 
remote_resource_to_local_path_transferor::to_string() const {
    return "transferor[remote_resource => local_path]";
}

} // namespace io
} // namespace norns
