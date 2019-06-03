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
