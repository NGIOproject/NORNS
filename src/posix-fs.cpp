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

#include <sstream>
#include <boost/foreach.hpp>
#include "backend-base.hpp"
#include "utils.hpp"
#include "posix-fs.hpp"

namespace storage {

posix_fs::posix_fs(const std::string& mount, uint32_t quota) 
    : m_mount(mount), m_quota(quota) { }

std::string posix_fs::mount() const {
    return m_mount;
}

uint32_t posix_fs::quota() const {
    return m_quota;
}

void posix_fs::read_data() const {
}

void posix_fs::write_data() const {
}

std::string posix_fs::to_string() const {
    std::stringstream ss;

    ss << "{NORNS_POSIX, " << m_mount << ", " << m_quota << "}";

    return ss.str();
}

} // namespace storage
