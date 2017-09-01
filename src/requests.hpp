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

#ifndef __REQUESTS_HPP__
#define __REQUESTS_HPP__

#include <cstdint>
#include <vector>
#include <string>

#include "backend-base.hpp"
#include "request-base.hpp"

using backend_ptr = std::shared_ptr<storage::backend>;

class job_registration_request : public urd_request {

public:
    job_registration_request(uint32_t jobid, 
                             const std::vector<std::string>& hosts, 
                             const std::vector<backend_ptr>& backends);
    uint32_t id() const;
    std::vector<std::string> hosts() const;
    std::vector<backend_ptr> backends() const;

    bool validate() const;
    std::string to_string() const;

private:
    uint32_t                    m_jobid;
    std::vector<std::string>    m_hosts;
    std::vector<backend_ptr>    m_backends;
};

class job_update_request : public urd_request {
public:
    job_update_request(uint32_t jobid, 
                       const std::vector<std::string>& hosts, 
                       const std::vector<backend_ptr>& backends);
    uint32_t id() const;
    std::vector<std::string> hosts() const;
    std::vector<backend_ptr> backends() const;

    bool validate() const;
    std::string to_string() const;

private:
    uint32_t                            m_jobid;
    std::vector<std::string>            m_hosts;
    std::vector<backend_ptr>    m_backends;
};

class job_removal_request : public urd_request {
public:
    job_removal_request(uint32_t jobid);
    uint32_t id() const;

    bool validate() const;
    std::string to_string() const;

private:
    uint32_t                    m_jobid;
};

#endif /* __REQUEST_HPP__ */
