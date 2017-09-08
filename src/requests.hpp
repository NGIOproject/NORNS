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

#ifndef __REQUESTS_HPP__
#define __REQUESTS_HPP__

#include <cstdint>
#include <vector>
#include <string>
#include <sys/types.h>

#include "backend-base.hpp"
#include "request-base.hpp"

using backend_ptr = std::shared_ptr<storage::backend>;

class bad_request : public urd_request {
public:
    bad_request() {}
    bool validate() const { 
        return true; 
    };

    std::string to_string() const {
        return "BAD_REQUEST";
    };
};

class job_registration_request : public urd_request {

public:
    job_registration_request(uint32_t jobid, 
                             const std::vector<std::string>& hosts, 
                             const std::vector<backend_ptr>& backends);
    uint32_t jobid() const;
    std::vector<std::string> hosts() const;
    std::vector<backend_ptr> backends() const;

    bool validate() const;
    std::string to_string() const;

private:
    uint32_t                    m_jobid;
    std::vector<std::string>    m_hosts;
    std::vector<backend_ptr>    m_backends;
};

class job_update_request : public urd_request {
public:
    job_update_request(uint32_t jobid, 
                       const std::vector<std::string>& hosts, 
                       const std::vector<backend_ptr>& backends);
    uint32_t jobid() const;
    std::vector<std::string> hosts() const;
    std::vector<backend_ptr> backends() const;

    bool validate() const;
    std::string to_string() const;

private:
    uint32_t                    m_jobid;
    std::vector<std::string>    m_hosts;
    std::vector<backend_ptr>    m_backends;
};

class job_removal_request : public urd_request {
public:
    job_removal_request(uint32_t jobid);
    uint32_t jobid() const;

    bool validate() const;
    std::string to_string() const;

private:
    uint32_t                    m_jobid;
};

class process_registration_request : public urd_request {
public:
    process_registration_request(uint32_t jobid, uid_t uid, gid_t gid, pid_t pid);

    uint32_t jobid() const;
    uid_t uid() const;
    gid_t gid() const;
    pid_t pid() const;

    bool validate() const;
    std::string to_string() const;

private:
    uint32_t m_jobid;
    uid_t m_uid;
    gid_t m_gid;
    pid_t m_pid;
};

class process_deregistration_request : public urd_request {
public:
    process_deregistration_request(uint32_t jobid, uid_t uid, gid_t gid, pid_t pid);

    uint32_t jobid() const;
    uid_t uid() const;
    gid_t gid() const;
    pid_t pid() const;

    bool validate() const;
    std::string to_string() const;

private:
    uint32_t m_jobid;
    uid_t m_uid;
    gid_t m_gid;
    pid_t m_pid;
};

struct resource {
};

struct memory_buffer : public resource {

    memory_buffer(uint32_t backend_type, uint64_t address, std::size_t size)
        : m_backend_type(backend_type),
          m_address(address),
          m_size(size) {}

    uint32_t m_backend_type;
    uint64_t m_address;
    std::size_t m_size;
};

struct filesystem_path : public resource {

    filesystem_path(uint32_t backend_type, std::string hostname, std::string datapath)
        : m_backend_type(backend_type),
          m_hostname(hostname),
          m_datapath(datapath) {}

    uint32_t m_backend_type;
    std::string m_hostname;
    std::string m_datapath;
};

class iotask_request : public urd_request {

public:
    iotask_request(uint32_t optype, const resource& src, const resource& dst) 
        : m_optype(optype),
          m_data_src(src), 
          m_data_dst(dst) { }

    bool validate() const { return true; }
    std::string to_string() const { return "path"; }

private:
    uint32_t m_optype;
    resource m_data_src;
    resource m_data_dst;
};

#endif /* __REQUEST_HPP__ */
