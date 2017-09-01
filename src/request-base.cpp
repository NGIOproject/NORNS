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

    urd_request* parsed_req = nullptr;
    norns::rpc::Request rpc_req;

    if(rpc_req.ParseFromArray(buffer.data(), size)) {
        switch(rpc_req.type()) {

            case norns::rpc::Request::SUBMIT_IOTASK:
                break;
            case norns::rpc::Request::REGISTER_JOB:
            case norns::rpc::Request::UPDATE_JOB:

                if(rpc_req.has_jobid() && rpc_req.has_job()) {

                    auto id = rpc_req.jobid();
                    auto job = rpc_req.job();
                    urd_request* req_ptr = nullptr;

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
                        req_ptr = new job_registration_request(id, hosts, backends);
                    }
                    else { // rpc_req.type() == norns::rpc::Request::UPDATE_JOB)
                        req_ptr = new job_update_request(id, hosts, backends);
                    }

                    parsed_req = req_ptr;
                }
                break;
            case norns::rpc::Request::UNREGISTER_JOB:

                if(rpc_req.has_jobid()) {
                    parsed_req = new job_removal_request(rpc_req.jobid());
                }

                break;
        }
    }

    return parsed_req;
}
