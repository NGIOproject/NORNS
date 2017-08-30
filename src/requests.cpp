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

#include "requests.hpp"
#include <iostream>

job_registration_request::job_registration_request(uint32_t jobid) 
 : m_jobid(jobid) { }

uint32_t job_registration_request::id() const {
    return m_jobid;
}

std::vector<std::string> job_registration_request::hosts() const {
    return m_hosts;
}

// std::vector<backend> job_registration_request::backends() const {
//     return m_backends;
// }

void job_registration_request::process() {

    std::cerr << std::dec << "Ho ho! " << m_jobid << "\n";
}
