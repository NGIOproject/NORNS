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

#include "backend-base.hpp"

namespace storage {

    backend_factory& backend_factory::get_instance() {
        static backend_factory _;
        return _;
    }

    std::shared_ptr<backend> backend_factory::create(int32_t type, const std::string& mount, uint32_t quota) const {

        const auto& it = m_registrar.find(type);

        if(it != m_registrar.end()){
            return std::shared_ptr<backend>(it->second(mount, quota));
        }
        else{
            throw std::invalid_argument("Unrecognized backend type!");
        }
    };

}
