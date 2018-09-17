/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of the NORNS Data Scheduler, a service that allows  *
 * other programs to start, track and manage asynchronous transfers of   *
 * data resources transfers requests between different storage backends. *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The NORNS Data Scheduler is free software: you can redistribute it    *
 * and/or modify it under the terms of the GNU Lesser General Public     *
 * License as published by the Free Software Foundation, either          *
 * version 3 of the License, or (at your option) any later version.      *
 *                                                                       *
 * The NORNS Data Scheduler is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with the NORNS Data Scheduler.  If not, see      *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

#include "resources.hpp"
#include "transferor-registry.hpp"

namespace norns {
namespace io {

bool 
transferor_registry::add(const data::resource_type t1, 
                         const data::resource_type t2, 
                         std::shared_ptr<io::transferor>&& trp) {

    using ValueType = std::shared_ptr<io::transferor>;

    if(m_transferors.count(std::make_pair(t1, t2)) == 0) {
        m_transferors.emplace(std::make_pair(t1, t2), 
                              std::forward<ValueType>(trp));
        return true;
    }

    return false;
}

std::shared_ptr<io::transferor> 
transferor_registry::get(const data::resource_type t1, 
                         const data::resource_type t2) const {

    using ValueType = std::shared_ptr<io::transferor>;

    if(m_transferors.count(std::make_pair(t1, t2)) == 0) {
        return ValueType();
    }

    return m_transferors.at(std::make_pair(t1, t2));
}

} // namespace io
} // namespace norns
