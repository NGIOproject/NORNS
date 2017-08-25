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

#include "request-base.hpp"

class register_job : public urd_request {

    struct backend {
        int32_t     m_type;
        std::string m_mount;
        int32_t m_quota;
    };

    friend class urd_request;

public:
    register_job(uint32_t id);
    void process();

private:
    uint32_t                    m_id;
    std::vector<std::string>    m_hosts;
    std::vector<backend>        m_backends;
};

#endif /* __REQUEST_HPP__ */
