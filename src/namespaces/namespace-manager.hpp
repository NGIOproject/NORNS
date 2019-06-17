/************************************************************************* 
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of NORNS, a service that allows other programs to   *
 * start, track and manage asynchronous transfers of data resources      *
 * between different storage backends.                                   *
 *                                                                       *
 * See AUTHORS file in the top level directory for information regarding *
 * developers and contributors.                                          *
 *                                                                       *
 * This software was developed as part of the EC H2020 funded project    *
 * NEXTGenIO (Project ID: 671951).                                       *
 *     www.nextgenio.eu                                                  *
 *                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining *
 * a copy of this software and associated documentation files (the       *
 * "Software"), to deal in the Software without restriction, including   *
 * without limitation the rights to use, copy, modify, merge, publish,   *
 * distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to *
 * the following conditions:                                             *
 *                                                                       *
 * The above copyright notice and this permission notice shall be        *
 * included in all copies or substantial portions of the Software.       *
 *                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                 *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS   *
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN    *
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN     *
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE      *
 * SOFTWARE.                                                             *
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
    contains(const std::string& nsid) const {
        return m_namespaces.count(nsid) != 0;
    }

    boost::optional<std::shared_ptr<storage::backend>>
    find(const std::string& nsid, bool is_remote = false) const {

        if(is_remote) {
            return static_cast<std::shared_ptr<storage::backend>>(
                    std::make_shared<storage::detail::remote_backend>(nsid));
        }

        if(m_namespaces.count(nsid) == 0) {
            return boost::none;
        }

        return m_namespaces.at(nsid);
    }

    std::tuple<bool, std::vector<std::shared_ptr<storage::backend>>>
    find(const std::vector<std::string>& nsids, 
         const std::vector<bool>& remotes) const {

        bool all_found = true;

        assert(nsids.size() == remotes.size());

        std::vector<std::shared_ptr<storage::backend>> v;

        for(std::size_t i=0; i<nsids.size(); ++i) {

            v.push_back(find(nsids[i], remotes[i]).get_value_or(nullptr));

            if(!v.back()) {
                all_found = false;
            }
        }

        return std::make_tuple(all_found, v);
    }

    template <typename UnaryPredicate>
    std::size_t
    count_if(UnaryPredicate&& p) const {

        using kv_type = std::pair<std::string, 
                                  std::shared_ptr<storage::backend>>;

        return std::count_if(
                m_namespaces.begin(),
                m_namespaces.end(), 
                [&](const kv_type& kv) {
                    return p(kv.second);
                }); 
    }


private:
    std::unordered_map<std::string, 
                       std::shared_ptr<storage::backend>> m_namespaces;

};

} // namespace ns
} // namespace norns

#endif /* __NAMESPACE_MANAGER_HPP__ */
