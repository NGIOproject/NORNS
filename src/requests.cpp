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
#include <sstream>
#include <boost/algorithm/string/join.hpp>
#include <boost/range/adaptor/transformed.hpp>

/****************************/
/* job_registration_request */
/****************************/

job_registration_request::job_registration_request(uint32_t jobid, const std::vector<std::string>& hosts, 
                                                   const std::vector<std::shared_ptr<storage::backend>>& backends)
 : m_jobid(jobid),
   m_hosts(hosts),
   m_backends(backends) { }


uint32_t job_registration_request::id() const {
    return m_jobid;
}

std::vector<std::string> job_registration_request::hosts() const {
    return m_hosts;
}

std::vector<backend_ptr> job_registration_request::backends() const {
    return m_backends;
}

bool job_registration_request::validate() const {
    return true;
}

std::string job_registration_request::to_string() const {

    using boost::algorithm::join;
    using boost::adaptors::transformed;

    std::stringstream ss;

    auto to_string = [](const backend_ptr& b) {
        return b->to_string();
    };

    ss << "id: " << m_jobid << ", "
       << "hosts: {" << join(m_hosts, ", ") << "}; "
       << "backends: {" << join(m_backends | transformed(to_string), ", ");

    return ss.str();
}

/**********************/
/* job_update_request */
/**********************/

job_update_request::job_update_request(uint32_t jobid, const std::vector<std::string>& hosts, 
                                       const std::vector<backend_ptr>& backends)
 : m_jobid(jobid),
   m_hosts(hosts),
   m_backends(backends) { }


uint32_t job_update_request::id() const {
    return m_jobid;
}

std::vector<std::string> job_update_request::hosts() const {
    return m_hosts;
}

std::vector<backend_ptr> job_update_request::backends() const {
    return m_backends;
}

bool job_update_request::validate() const {
    return true;
}

std::string job_update_request::to_string() const {

    using boost::algorithm::join;
    using boost::adaptors::transformed;

    std::stringstream ss;

    auto to_string = [](const backend_ptr& b) {
        return b->to_string();
    };

    ss << "id: " << m_jobid << ", "
       << "hosts: {" << join(m_hosts, ", ") << "}; "
       << "backends: {" << join(m_backends | transformed(to_string), ", ");

    return ss.str();
}

/**********************/
/* job_removal_request */
/**********************/

job_removal_request::job_removal_request(uint32_t jobid)
 : m_jobid(jobid) { }

uint32_t job_removal_request::id() const {
    return m_jobid;
}

bool job_removal_request::validate() const {
    return true;
}

std::string job_removal_request::to_string() const {

    return "id: " + std::to_string(m_jobid);
}


