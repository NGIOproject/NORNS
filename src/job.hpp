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

#ifndef __JOB_HPP__
#define __JOB_HPP__

#include <vector>
#include <map>

#include "backends.hpp"


namespace norns {

struct process { }; // temporary stub, will probably replace with a valid munge credential

class job {

    using backend_ptr = std::shared_ptr<storage::backend>;
    using process_ptr = std::shared_ptr<process>;

public:
    job(uint32_t jobid, const std::vector<std::string>& hosts)
        : m_jobid(jobid),
          m_hosts(hosts) {}

    void update(const std::vector<std::string>& hosts) {
        m_hosts = hosts;
    }

    void add_process(pid_t pid, gid_t gid) {

        auto key = std::make_pair(pid, gid);

        const auto& it = m_processes.find(key);

        if(it == m_processes.end()) {
            m_processes.emplace(key, std::make_shared<process>());
        }
    }

    bool find_and_remove_process(pid_t pid, gid_t gid) {

        auto key = std::make_pair(pid, gid);

        const auto& it = m_processes.find(key);

        if(it == m_processes.end()) {
            return false;
        }

        m_processes.erase(it);
        return true;
    }

private:
    uint32_t                                   m_jobid;
    std::vector<std::string>                   m_hosts;
    std::map<int32_t, backend_ptr>             m_backends;
    std::map<std::pair<pid_t, gid_t>, process_ptr> m_processes;
}; 

} // namespace norns

#endif /* __JOB_HPP__ */
