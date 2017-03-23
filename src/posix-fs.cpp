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

#include <boost/foreach.hpp>
#include "backends.hpp"
#include "utils.hpp"
#include "posix-fs.hpp"

namespace storage {

REGISTER_BACKEND("posix-fs", posix_fs);

posix_fs::posix_fs(const bpt::ptree& options) {

    // parse options
    for(const auto& kv : options) {

        if(kv.first == "name"){
            m_name = kv.second.get_value<std::string>();
        }

        if(kv.first == "type"){
            m_type = kv.second.get_value<std::string>();
        }

        if(kv.first == "description"){
            m_description = kv.second.get_value<std::string>();
        }
        
        if(kv.first == "capacity"){
            std::string capacity_str = kv.second.get_value<std::string>();

            try {
                m_capacity = utils::parse_size(capacity_str);
            } catch(std::invalid_argument ex) {
                std::cerr << "Error parsing backend 'capacity' parameter: '" << capacity_str << "': " << ex.what() << "\n";
                exit(EXIT_FAILURE);
            }
        }
    }
}

const std::string& posix_fs::get_name() const {
    return m_name;
}

const std::string& posix_fs::get_type() const {
    return m_type;
}

const std::string& posix_fs::get_description() const {
    return m_description;
}

uint64_t posix_fs::get_capacity() const {
    return m_capacity;
}

void posix_fs::read_data() const {
}

void posix_fs::write_data() const {
}

} // namespace storage
