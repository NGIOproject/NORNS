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

#ifndef __POSIX_FS_HPP__
#define __POSIX_FS_HPP__

#include "backends.hpp"

namespace bpt = boost::property_tree;

namespace storage {

class posix_fs final : public storage::backend {
public:
    posix_fs(const bpt::ptree& options);

    const std::string& get_name() const override;
    const std::string& get_type() const override;
    const std::string& get_description() const override;
    uint64_t get_capacity() const override;
    void read_data() const override;
    void write_data() const override;

private:
    std::string m_name;
    std::string m_type;
    std::string m_description;
    uint64_t    m_capacity;

};

} // namespace storage

#endif // __POSIX_FS_HPP__
