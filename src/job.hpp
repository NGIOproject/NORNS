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

#ifndef __JOB_HPP__
#define __JOB_HPP__

#include <vector>
#include <map>

#include "backend-base.hpp"

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

    void remove_process(pid_t pid, gid_t gid) {

        auto key = std::make_pair(pid, gid);

        const auto& it = m_processes.find(key);

        if(it != m_processes.end()) {
            m_processes.erase(it);
        }
    }

private:
    uint32_t                                   m_jobid;
    std::vector<std::string>                   m_hosts;
    std::map<int32_t, backend_ptr>             m_backends;
    std::map<std::pair<pid_t, gid_t>, process_ptr> m_processes;
}; 

#endif /* __JOB_HPP__ */
