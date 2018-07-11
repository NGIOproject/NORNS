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

#include <sstream>
#include <boost/algorithm/string/join.hpp>

#include "common.hpp"
#include "messages.pb.h"
#include "backends.hpp"
#include "resources.hpp"
#include "utils.hpp"
#include "request.hpp"

namespace {

norns::iotask_type decode_iotask_type(::google::protobuf::uint32 type) {

    using norns::iotask_type;

    switch(type) {
        case NORNS_IOTASK_COPY:
            return iotask_type::copy;
        case NORNS_IOTASK_MOVE:
            return iotask_type::move;
        default:
            return iotask_type::unknown;
    }
}

norns::backend_type decode_backend_type(::google::protobuf::uint32 type) {

    using norns::backend_type;

    switch(type) {
        case NORNS_BACKEND_NVML:
            return backend_type::nvml;
        case NORNS_BACKEND_LUSTRE:
            return backend_type::lustre;
//        //XXX PROCESS_MEMORY should never be received
//        case NORNS_BACKEND_PROCESS_MEMORY: // deprecated
//            return backend_type::process_memory;
        case NORNS_BACKEND_ECHOFS:
            return backend_type::echofs;
        case NORNS_BACKEND_POSIX_FILESYSTEM:
            return backend_type::posix_filesystem;
        default:
            return backend_type::unknown;
    }
}


bool is_valid(const norns::rpc::Request_Task_Resource& res) {
    if(!(res.type() & (NORNS_PROCESS_MEMORY | NORNS_POSIX_PATH))) {
        return false;
    }

    if(res.type() & NORNS_PROCESS_MEMORY) { 
        if(!res.has_buffer()) { 
            return false;
        }

        return true;
    }

    if(res.type() & NORNS_POSIX_PATH) {
        if(!res.has_path()) { 
            return false;
        }

        if(!(res.type() & (R_LOCAL | R_SHARED | R_REMOTE))) {
            return false;
        }

        if((res.type() & R_LOCAL) && res.path().has_hostname()) {
            return false;
        }

        if((res.type() & R_SHARED) && res.path().has_hostname()) {
            return false;
        }

        if((res.type() & R_REMOTE) && !res.path().has_hostname()) {
            return false;
        }

        return true;
    }

    return true;
}

std::shared_ptr<norns::data::resource_info> 
create_from(const norns::rpc::Request_Task_Resource& res) {

    using norns::data::resource_info;
    using norns::data::memory_buffer;
    using norns::data::local_path_info;
    using norns::data::shared_path_info;
    using norns::data::remote_path_info;

    if(is_valid(res)) {
        if(res.type() & NORNS_PROCESS_MEMORY) {
            return std::make_shared<memory_buffer>(res.buffer().address(),
                                                   res.buffer().size());
        }
        else { // NORNS_POSIX_PATH
            if(res.type() & R_LOCAL) {
                return std::make_shared<local_path_info>(res.path().nsid(),
                                                         res.path().datapath());
            }
            else if(res.type() & R_SHARED) {
                return std::make_shared<shared_path_info>(res.path().nsid(),
                                                          res.path().datapath());
            }
            else { // R_REMOTE
                return std::make_shared<remote_path_info>(res.path().nsid(),
                                                          res.path().hostname(), 
                                                          res.path().datapath());
            }
        }
    }

    return std::shared_ptr<resource_info>();
}

std::string 
to_string(const std::vector<std::shared_ptr<norns::storage::backend>>& blist) {

    std::string str;

    for(const auto& b : blist) {
        str += b->to_string() + ", ";
    }

    if(str.length() != 0) {
        str.pop_back();
        str.pop_back();
    }

    return str;
}

}

namespace norns {
namespace api {

using request_ptr = std::unique_ptr<request>;

request_ptr request::create_from_buffer(const std::vector<uint8_t>& buffer, int size) {

    norns::rpc::Request rpc_req;

    if(rpc_req.ParseFromArray(buffer.data(), size)) {
        switch(rpc_req.type()) {
            case norns::rpc::Request::IOTASK_SUBMIT:

                if(rpc_req.has_task()) {

                    auto task = rpc_req.task();
                    iotask_type optype = ::decode_iotask_type(task.optype());

                    if(optype == iotask_type::unknown) {
                        return std::make_unique<bad_request>();
                    }

                    std::shared_ptr<data::resource_info> src_res = ::create_from(task.source());
                    std::shared_ptr<data::resource_info> dst_res = ::create_from(task.destination());

                    if(src_res != nullptr && dst_res != nullptr) {
                        return std::make_unique<iotask_create_request>(optype, src_res, dst_res);
                    }

                    return std::make_unique<bad_request>();
                }
            break;

            case norns::rpc::Request::IOTASK_STATUS:
                if(rpc_req.has_task()) {

                    auto task = rpc_req.task();

                    if(task.has_taskid()) {
                        return std::make_unique<iotask_status_request>(task.taskid());
                    }
                }
            break;

            case norns::rpc::Request::PING:
                return std::make_unique<ping_request>();

            case norns::rpc::Request::CTL_STATUS:
                return std::make_unique<ctl_status_request>();

            case norns::rpc::Request::JOB_REGISTER:
            case norns::rpc::Request::JOB_UPDATE:

                if(rpc_req.has_jobid() && rpc_req.has_job()) {

                    auto id = rpc_req.jobid();
                    auto job = rpc_req.job();

                    std::vector<std::string> hosts;
                    hosts.reserve(job.hosts().size());

                    for(const auto& h : job.hosts()) {
                        hosts.push_back(h);
                    }

                    std::vector<std::shared_ptr<storage::backend>> backends;
#if 0 
                    //XXX deprecated for now, backends are registered elsewhere
                    //here we should only propagate access rights to backends,
                    // i.e. nsids this job can access and usage quotas
                    backends.reserve(job.backends().size());


                    for(const auto& b : job.backends()) {
                        backends.push_back(storage::backend_factory::create_from(b.type(), b.mount(), b.quota()));
                    }
#endif

                    //XXX TODO instantiate job_capabilities
                    //and pass them to make_unique<XXX> below
                    //instead of 'backends'

                    if(rpc_req.type() == norns::rpc::Request::JOB_REGISTER) {
                        return std::make_unique<job_register_request>(id, hosts, backends);
                    }
                    else { // rpc_req.type() == norns::rpc::Request::JOB_UPDATE)
                        return std::make_unique<job_update_request>(id, hosts, backends);
                    }
                }
                break;

            case norns::rpc::Request::JOB_UNREGISTER:
                if(rpc_req.has_jobid()) {
                    return std::make_unique<job_unregister_request>(rpc_req.jobid());
                }
                break;

            case norns::rpc::Request::PROCESS_ADD:
            case norns::rpc::Request::PROCESS_REMOVE:
                if(rpc_req.has_jobid() && rpc_req.has_process()) {

                    auto process = rpc_req.process();

                    if(rpc_req.type() == norns::rpc::Request::PROCESS_ADD) {
                        return std::make_unique<process_register_request>(
                                rpc_req.jobid(), 
                                process.uid(), 
                                process.gid(), 
                                process.pid());
                    }
                    else { // rpc_req.type() == norns::rpc::Request::PROCESS_REMOVE
                        return std::make_unique<process_unregister_request>(
                                rpc_req.jobid(), 
                                process.uid(), 
                                process.gid(), 
                                process.pid());
                    }
                }
                break;

            case norns::rpc::Request::NAMESPACE_REGISTER:
            case norns::rpc::Request::NAMESPACE_UPDATE:
            case norns::rpc::Request::NAMESPACE_UNREGISTER:
                if(rpc_req.has_nspace()) {

                    auto nspace = rpc_req.nspace();

                    if(nspace.has_backend()) {

                        const auto& b = nspace.backend();
                        backend_type type = ::decode_backend_type(b.type());

                        if(type == backend_type::unknown) {
                            break;
                        }

                        if(rpc_req.type() == norns::rpc::Request::NAMESPACE_REGISTER) {
                            return std::make_unique<backend_register_request>(nspace.nsid(), 
                                                                              type, 
                                                                              b.mount(), 
                                                                              b.capacity());
                        }
                        else { // rpc_req.type() == norns::rpc::Request::NAMESPACE_UPDATE
                            return std::make_unique<backend_update_request>(nspace.nsid(), 
                                                                            type, 
                                                                            b.mount(), 
                                                                            b.capacity());
                        }
                    }
                    else { // norns::rpc::Request::NAMESPACE_UNREGISTER
                        return std::make_unique<backend_unregister_request>(nspace.nsid());
                    }
                }
                break;
        }
    }

    return std::make_unique<bad_request>();
}

void request::cleanup() {
    google::protobuf::ShutdownProtobufLibrary();
}

namespace detail {

template<>
std::string bad_request::to_string() const {
    return "BAD_REQUEST";
}

template<>
std::string job_register_request::to_string() const {

    using boost::algorithm::join;
    std::stringstream ss;

    const auto& jobid = this->get<0>();
    const auto& hosts = this->get<1>();
    const auto& backends = this->get<2>();

    ss << "jobid: " << jobid << ", "
       << "hosts: {" << join(hosts, ", ") << "}; "
       << "backends: {" << ::to_string(backends) << "}";

    return ss.str();
}

template<>
std::string job_update_request::to_string() const {

    using boost::algorithm::join;
    std::stringstream ss;

    const auto& jobid = this->get<0>();
    const auto& hosts = this->get<1>();
    const auto& backends = this->get<2>();

    ss << "jobid: " << jobid << ", "
       << "hosts: {" << join(hosts, ", ") << "}; "
       << "backends: {" << ::to_string(backends) << "}";

    return ss.str();
}

template<>
std::string job_unregister_request::to_string() const {

    const auto& jobid = this->get<0>();

    return "jobid: " + std::to_string(jobid);
}

template<>
std::string process_register_request::to_string() const {

    const auto& jobid = this->get<0>();
    const auto& uid = this->get<1>();
    const auto& gid = this->get<2>();
    const auto& pid = this->get<3>();

    return "jobid: " + std::to_string(jobid) + ", "
           "uid:" + std::to_string(uid) + ", "
           "gid:" + std::to_string(gid) + ", "
           "pid:" + std::to_string(pid);
}

template<>
std::string process_unregister_request::to_string() const {

    const auto& jobid = this->get<0>();
    const auto& uid = this->get<1>();
    const auto& gid = this->get<2>();
    const auto& pid = this->get<3>();

    return "jobid: " + std::to_string(jobid) + ", "
           "uid:" + std::to_string(uid) + ", "
           "gid:" + std::to_string(gid) + ", "
           "pid:" + std::to_string(pid);
}

template<>
std::string backend_register_request::to_string() const {

    const auto& nsid = this->get<0>();
    const auto& type = this->get<1>();
    const auto& mount = this->get<2>();
    const auto& quota = this->get<3>();

#if 0 // verbose
    return "nsid: \"" + nsid + "\", "
           "type:" + utils::to_string(type) + ", "
           (mount != "" ?  "mount: \"" + mount + "\", " : "") +
           (quota != 0 ? "quota: " + std::to_string(quota) : "unlimited");
#else
    return "\"" + nsid + "\", " +
           utils::to_string(type) + ", " +
           (mount != "" ?  "\"" + mount + "\", " : "") +
           (quota != 0 ? std::to_string(quota) : "unlimited");
#endif
}

template<>
std::string backend_update_request::to_string() const {

    const auto& nsid = this->get<0>();
    const auto& type = this->get<1>();
    const auto& mount = this->get<2>();
    const auto& quota = this->get<3>();

#if 0 // verbose
    return "nsid: \"" + nsid + "\", "
           "type:" + utils::to_string(type) + ", "
           (mount != "" ?  "mount: \"" + mount + "\", " : "") +
           (quota != 0 ? "quota: " + std::to_string(quota) : "unlimited");
#else
    return "\"" + nsid + "\", " +
           utils::to_string(type) + ", " +
           (mount != "" ?  "\"" + mount + "\", " : "") +
           (quota != 0 ? std::to_string(quota) : "unlimited");
#endif
}

template<>
std::string backend_unregister_request::to_string() const {

    const auto& nsid = this->get<0>();
    
    return "\"" + nsid + "\"";
}

template<>
std::string iotask_create_request::to_string() const {

    const auto op = this->get<0>();
    const auto src = this->get<1>();
    const auto dst = this->get<2>();

    return utils::to_string(op) + ", "
           + src->to_string() + " => "
           + dst->to_string();
}

template<>
std::string iotask_status_request::to_string() const {

    const auto tid = this->get<0>();

    return std::to_string(tid);
}

} // namespace detail


} // namespace api
} // namespace norns



