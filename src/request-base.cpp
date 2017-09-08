//
// Copyright (C) 2017 Barcelona Supercomputing Center
//                    Centro Nacional de Supercomputacion
//
// This file is part of the Data Scheduler, a daemon for tracking and managing
// requests for asynchronous data transfer in a hierarchical storage environment.
//
// See AUTHORS file in the top level directory for information
// regarding developers and contributors.
//
// The Data Scheduler is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Data Scheduler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Data Scheduler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "messages.pb.h"
#include "backend-base.hpp"
#include "request-base.hpp"
#include "requests.hpp"
#include <iostream>

urd_request* urd_request::create_from_buffer(const std::vector<uint8_t>& buffer, int size) {

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

                        return new iotask_request(optype, src_buf, dst_path);
                    }

                    if(src.has_path()) {
                        filesystem_path src_path(src.type(), src.path().hostname(), src.path().datapath());
                        filesystem_path dst_path(dst.type(), dst.path().hostname(), dst.path().datapath());

                        return new iotask_request(optype, src_path, dst_path);
                    }

                    return new bad_request();
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
                        return new job_registration_request(id, hosts, backends);
                    }
                    else { // rpc_req.type() == norns::rpc::Request::UPDATE_JOB)
                        return new job_update_request(id, hosts, backends);
                    }
                }
                break;

            case norns::rpc::Request::UNREGISTER_JOB:
                if(rpc_req.has_jobid()) {
                    return new job_removal_request(rpc_req.jobid());
                }
                break;

            case norns::rpc::Request::ADD_PROCESS:
            case norns::rpc::Request::REMOVE_PROCESS:
                if(rpc_req.has_jobid() && rpc_req.has_process()) {

                    auto process = rpc_req.process();

                    if(rpc_req.type() == norns::rpc::Request::ADD_PROCESS) {
                        return new process_registration_request(rpc_req.jobid(), process.uid(), 
                                                                   process.gid(), process.pid());
                    }
                    else { // rpc_req.type() == norns::rpc::Request::REMOVE_PROCESS
                        return new process_deregistration_request(rpc_req.jobid(), process.uid(), 
                                                                  process.gid(), process.pid());
                    }
                }
                break;
        }
    }

    return new bad_request();
}
