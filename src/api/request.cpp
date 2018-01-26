/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 *                                                                       *
 * This file is part of the Data Scheduler, a daemon for tracking and    *
 * managing requests for asynchronous data transfer in a hierarchical    *
 * storage environment.                                                  *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The Data Scheduler is free software: you can redistribute it and/or   *
 * modify it under the terms of the GNU Lesser General Public License    *
 * as published by the Free Software Foundation, either version 3 of     *
 * the License, or (at your option) any later version.                   *
 *                                                                       *
 * The Data Scheduler is distributed in the hope that it will be useful, *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with Data Scheduler.  If not, see                *
 * <http://www.gnu.org/licenses/>.                                       *
 *                                                                       *
 *************************************************************************/

#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include "make-unique.hpp"
#include "messages.pb.h"
#include "backend-base.hpp"
#include "request.hpp"

namespace api {

using request_ptr = std::unique_ptr<request>;

request_ptr request::create_from_buffer(const std::vector<uint8_t>& buffer, int size) {

    norns::rpc::Request rpc_req;

    if(rpc_req.ParseFromArray(buffer.data(), size)) {
        switch(rpc_req.type()) {
            case norns::rpc::Request::SUBMIT_IOTASK:

                if(rpc_req.has_task()) {

                    auto task = rpc_req.task();
                    auto optype = task.optype();
                    auto src = task.source();
                    auto dst = task.destination();

                    if(src.has_buffer()) {
                        memory_buffer src_buf(src.type(), src.buffer().address(), src.buffer().size());
                        filesystem_path dst_path(dst.type(), dst.path().hostname(), dst.path().datapath());

                        return std::make_unique<transfer_task_request>(optype, src_buf, dst_path);
                    }

                    if(src.has_path()) {
                        filesystem_path src_path(src.type(), src.path().hostname(), src.path().datapath());
                        filesystem_path dst_path(dst.type(), dst.path().hostname(), dst.path().datapath());

                        return std::make_unique<transfer_task_request>(optype, src_path, dst_path);
                    }

                    return std::make_unique<bad_request>();
                }
            break;

            case norns::rpc::Request::REGISTER_JOB:
            case norns::rpc::Request::UPDATE_JOB:

                if(rpc_req.has_jobid() && rpc_req.has_job()) {

                    auto id = rpc_req.jobid();
                    auto job = rpc_req.job();

                    std::vector<std::string> hosts;
                    hosts.reserve(job.hosts().size());

                    for(const auto& h : job.hosts()) {
                        hosts.push_back(h);
                    }

                    std::vector<std::shared_ptr<storage::backend>> backends;
                    backends.reserve(job.backends().size());


                    for(const auto& b : job.backends()) {
                        backends.push_back(storage::backend_factory::create_from(b.type(), b.mount(), b.quota()));
                    }

                    if(rpc_req.type() == norns::rpc::Request::REGISTER_JOB) {
                        return std::make_unique<job_register_request>(id, hosts, backends);
                    }
                    else { // rpc_req.type() == norns::rpc::Request::UPDATE_JOB)
                        return std::make_unique<job_update_request>(id, hosts, backends);
                    }
                }
                break;

            case norns::rpc::Request::UNREGISTER_JOB:
                if(rpc_req.has_jobid()) {
                    return std::make_unique<job_unregister_request>(rpc_req.jobid());
                }
                break;

            case norns::rpc::Request::ADD_PROCESS:
            case norns::rpc::Request::REMOVE_PROCESS:
                if(rpc_req.has_jobid() && rpc_req.has_process()) {

                    auto process = rpc_req.process();

                    if(rpc_req.type() == norns::rpc::Request::ADD_PROCESS) {
                        return std::make_unique<process_register_request>(
                                rpc_req.jobid(), 
                                process.uid(), 
                                process.gid(), 
                                process.pid());
                    }
                    else { // rpc_req.type() == norns::rpc::Request::REMOVE_PROCESS
                        return std::make_unique<process_unregister_request>(
                                rpc_req.jobid(), 
                                process.uid(), 
                                process.gid(), 
                                process.pid());
                    }
                }
                break;

            case norns::rpc::Request::PING:
                return std::make_unique<ping_request>();
        }
    }

    return std::make_unique<bad_request>();
}

void request::cleanup() {
    google::protobuf::ShutdownProtobufLibrary();
}

namespace detail {

template<>
std::string bad_request::to_string() const {
    return "BAD_REQUEST";
}

template<>
std::string job_register_request::to_string() const {

    std::stringstream ss;

    const auto& jobid = this->get<0>();
    const auto& hosts = this->get<1>();
    const auto& backends = this->get<2>();

    using boost::algorithm::join;
    using boost::adaptors::transformed;

    auto to_string = [](const backend_ptr& b) {
        return b->to_string();
    };

    ss << "jobid: " << jobid << ", "
       << "hosts: {" << join(hosts, ", ") << "}; "
       << "backends: {" << join(backends | transformed(to_string), ", ");

    return ss.str();
}

template<>
std::string job_update_request::to_string() const {

    std::stringstream ss;

    const auto& jobid = this->get<0>();
    const auto& hosts = this->get<1>();
    const auto& backends = this->get<2>();

    using boost::algorithm::join;
    using boost::adaptors::transformed;

    auto to_string = [](const backend_ptr& b) {
        return b->to_string();
    };

    ss << "jobid: " << jobid << ", "
       << "hosts: {" << join(hosts, ", ") << "}; "
       << "backends: {" << join(backends | transformed(to_string), ", ");

    return ss.str();
}

template<>
std::string job_unregister_request::to_string() const {

    const auto& jobid = this->get<0>();

    return "jobid: " + std::to_string(jobid);
}

template<>
std::string process_register_request::to_string() const {

    const auto& jobid = this->get<0>();
    const auto& uid = this->get<1>();
    const auto& gid = this->get<2>();
    const auto& pid = this->get<3>();

    return "jobid: " + std::to_string(jobid) + ", "
           "uid:" + std::to_string(uid) + ", "
           "gid:" + std::to_string(gid) + ", "
           "pid:" + std::to_string(pid);
}

template<>
std::string process_unregister_request::to_string() const {

    const auto& jobid = this->get<0>();
    const auto& uid = this->get<1>();
    const auto& gid = this->get<2>();
    const auto& pid = this->get<3>();

    return "jobid: " + std::to_string(jobid) + ", "
           "uid:" + std::to_string(uid) + ", "
           "gid:" + std::to_string(gid) + ", "
           "pid:" + std::to_string(pid);
}

template<>
std::string transfer_task_request::to_string() const {

    return "TODO: ???";
}

} // namespace detail


} // namespace api



