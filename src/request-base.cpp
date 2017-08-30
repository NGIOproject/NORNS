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

                if(rpc_req.has_job()) {

                    auto job = rpc_req.job();
                    auto req_ptr = new job_registration_request(job.id());

                    req_ptr->m_hosts.reserve(job.hosts().size());

                    for(const auto& h : job.hosts()) {
                        req_ptr->m_hosts.push_back(h);
                    }

                    req_ptr->m_backends.reserve(job.backends().size());

                    for(const auto& b : job.backends()) {
                        req_ptr->m_backends.push_back({
                            b.type(),
                            b.mount(),
                            b.quota()
                        });
                    }

                    parsed_req = req_ptr;
                }
            break;
        }
    }

    return parsed_req;
}
