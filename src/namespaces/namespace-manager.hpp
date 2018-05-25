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

#ifndef __NAMESPACE_MANAGER_HPP__
#define __NAMESPACE_MANAGER_HPP__

#include <unordered_map>
#include <memory>
#include <boost/optional.hpp>

namespace norns {

// forward declarations

namespace storage {
class backend;
}

namespace ns {

struct namespace_manager {

    template <typename... Args>
    bool
    add(const std::string& nsid, Args&&... args) {

        if(m_namespaces.count(nsid) != 0) {
            return false;
        }

        m_namespaces.emplace(nsid, std::forward<Args>(args)...);

        return true;
    }

    template <typename... Args>
    bool 
    add(std::string&& nsid, Args&&... args) {

        if(m_namespaces.count(nsid) != 0) {
            return false;
        }

        m_namespaces.emplace(std::forward<std::string>(nsid), 
                             std::forward<Args>(args)...);

        return true;
    }

    bool
    remove(const std::string& nsid) {

        const auto& it = m_namespaces.find(nsid);

        if(it == m_namespaces.end()) {
            return false;
        }

        m_namespaces.erase(it);
        return true;
    }

    bool
    contains(const std::string& nsid) {
        return m_namespaces.count(nsid) != 0;
    }

    boost::optional<std::shared_ptr<storage::backend>>
    find(const std::string& nsid, bool is_remote = false) {

        if(is_remote) {
            return static_cast<std::shared_ptr<storage::backend>>(
                    std::make_shared<storage::detail::remote_backend>());
        }

        if(m_namespaces.count(nsid) == 0) {
            return boost::none;
        }

        return m_namespaces.at(nsid);
    }


private:
    std::unordered_map<std::string, 
                       std::shared_ptr<storage::backend>> m_namespaces;

};

} // namespace ns
} // namespace norns

#endif /* __NAMESPACE_MANAGER_HPP__ */
