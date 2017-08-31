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

#ifndef __RESPONSES_HPP__
#define __RESPONSES_HPP__

#include <cstdint>
#include <vector>
#include <string>

#include "response-base.hpp"

class job_registration_response : public urd_response {

public:
    job_registration_response();
    job_registration_response(uint32_t error_code);

    void set_error_code(uint32_t error_code);
    bool store_to_buffer(std::vector<uint8_t>& buffer);
    std::string to_string() const;

private:
    uint32_t                    m_error_code;
};

#endif /* __RESPONSES_HPP__ */
