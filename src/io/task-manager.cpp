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

#include <boost/optional.hpp>
#include <limits>
#include <numeric>

#include "task-stats.hpp"
#include "task-info.hpp"
#include "common.hpp"
#include "logger.hpp"
#include "task-manager.hpp"

namespace norns {
namespace io {

task_manager::task_manager(uint32_t nrunners, 
                           uint32_t backlog_size, 
                           bool dry_run,
                           uint32_t dry_run_duration) :
    m_backlog_size(backlog_size),
    m_dry_run(dry_run),
    m_dry_run_duration(dry_run_duration),
    m_runners(nrunners) {}

bool
task_manager::register_transfer_plugin(const data::resource_type t1,
                                       const data::resource_type t2,
                                       std::shared_ptr<io::transferor>&& trp) {

    return m_transferor_registry.add(t1, t2,
                    std::forward<std::shared_ptr<io::transferor>>(trp));
}

/// boost::optional<iotask_id>
/// task_manager::create_task(iotask_type type, const auth::credentials& auth,
///         const backend_ptr src_backend, const resource_info_ptr src_rinfo, 
///         const backend_ptr dst_backend, const resource_info_ptr dst_rinfo,
///         const transferor_ptr&& tx_ptr) {
/// 
///     boost::unique_lock<boost::shared_mutex> lock(m_mutex);
/// 
///     iotask_id tid = ++m_id_base;
/// 
///     if(m_task_info.count(tid) != 0) {
///         --m_id_base;
///         return boost::none;
///     }
/// 
///     auto it = m_task_info.end();
///     std::tie(it, std::ignore) = m_task_info.emplace(tid,
///             std::make_shared<task_info>(tid, type, auth, src_backend, src_rinfo,
///                                         dst_backend, dst_rinfo));
/// 
///     const std::shared_ptr<task_info> task_info_ptr = it->second;
/// 
///     // helper lambda to register the completion of tasks so that we can keep track
///     // of the consumed bandwidth by each task
///     const auto register_completion = [=]() {
///         assert(task_info_ptr->status() == task_status::finished ||
///                task_info_ptr->status() == task_status::finished_with_error);
/// 
///         LOGGER_DEBUG("Task {} finished [{} MiB/s]", 
///                 task_info_ptr->id(), task_info_ptr->bandwidth());
/// 
///         auto bw = task_info_ptr->bandwidth();
/// 
///         // bw might be nan if the task did not finish correctly
///         if(!std::isnan(bw)) {
/// 
///             const auto key = std::make_pair(task_info_ptr->src_rinfo()->nsid(),
///                                             task_info_ptr->dst_rinfo()->nsid());
/// 
///             if(!m_bandwidth_backlog.count(key)) {
///                 m_bandwidth_backlog.emplace(key, 
///                         boost::circular_buffer<double>(m_backlog_size));
///             }
/// 
///             m_bandwidth_backlog.at(key).push_back(bw);
///         }
///     };
/// 
///     if(!m_dry_run) {
///         switch(type) {
///             case iotask_type::copy:
///                 m_runners.submit_with_epilog_and_forget(
///                     io::task<iotask_type::copy>(
///                         std::move(task_info_ptr), std::move(tx_ptr)), register_completion);
///                 break;
///             case iotask_type::move:
///                 m_runners.submit_with_epilog_and_forget(
///                     io::task<iotask_type::move>(
///                         std::move(task_info_ptr), std::move(tx_ptr)), register_completion);
///                 break;
///             case iotask_type::remove:
///                 m_runners.submit_with_epilog_and_forget(
///                     io::task<iotask_type::remove>(
///                         std::move(task_info_ptr), std::move(tx_ptr)), register_completion);
///                 break;
///             default:
///                 m_runners.submit_and_forget(
///                     io::task<iotask_type::unknown>(
///                         std::move(task_info_ptr), std::move(tx_ptr)));
///         }
///     }
///     else {
///         m_runners.submit_and_forget(
///             io::task<iotask_type::noop>(std::move(task_info_ptr), std::move(tx_ptr)));
///     }
/// 
///     return tid;
/// }


std::tuple<urd_error, boost::optional<iotask_id>>
task_manager::create_task(iotask_type type, 
                          const auth::credentials& auth,
                          const std::vector<backend_ptr>& backend_ptrs,
                          const std::vector<resource_info_ptr>& rinfo_ptrs) {

    assert(backend_ptrs.size() == rinfo_ptrs.size());

    boost::unique_lock<boost::shared_mutex> lock(m_mutex);

    // fetch an iotask_id for this task
    iotask_id tid = ++m_id_base;

    if(m_task_info.count(tid) != 0) {
        --m_id_base;
        return std::make_tuple(urd_error::too_many_tasks, -1);
    }

    // immediately-invoked lambda to create the appropriate metadata for the task
    // and register it into m_task_info
    const auto task_info_ptr = [&]() {

        assert(backend_ptrs.size() == 1 || backend_ptrs.size() == 2);

        const backend_ptr src_backend = backend_ptrs[0];
        const resource_info_ptr src_rinfo = rinfo_ptrs[0];
        const backend_ptr dst_backend = (backend_ptrs.size() == 1 ? 
                                            nullptr : backend_ptrs[1]);
        const resource_info_ptr dst_rinfo = (rinfo_ptrs.size() == 1 ? 
                                            nullptr : rinfo_ptrs[1]);

        auto it = m_task_info.end();
        std::tie(it, std::ignore) = m_task_info.emplace(tid,
                std::make_shared<task_info>(tid, type, auth, 
                                            src_backend, src_rinfo,
                                            dst_backend, dst_rinfo));
        return it->second;
    }();

    // helper lambda to register the completion of tasks so that we can keep track
    // of the consumed bandwidth by each task
    // N.B: we use capture-by-value here so that the task_info_ptr is valid when 
    // the callback is invoked.
    const auto register_completion = [=]() {
        assert(task_info_ptr->status() == task_status::finished ||
               task_info_ptr->status() == task_status::finished_with_error);

        LOGGER_DEBUG("Task {} finished [{} MiB/s]", 
                task_info_ptr->id(), task_info_ptr->bandwidth());

        auto bw = task_info_ptr->bandwidth();

        // bw might be nan if the task did not finish correctly
        if(!std::isnan(bw)) {

            const auto key = std::make_pair(task_info_ptr->src_rinfo()->nsid(),
                                            task_info_ptr->dst_rinfo()->nsid());

            if(!m_bandwidth_backlog.count(key)) {
                m_bandwidth_backlog.emplace(key, 
                        boost::circular_buffer<double>(m_backlog_size));
            }

            m_bandwidth_backlog.at(key).push_back(bw);
        }
    };

    if(m_dry_run) {
        type = iotask_type::noop;
    }

    switch(type) {
        case iotask_type::remove:
        {
            assert(backend_ptrs.size() == 1);

            m_runners.submit_and_forget(
                io::task<iotask_type::remove>(std::move(task_info_ptr)));
            break;
        }

        case iotask_type::copy:
        {
            assert(backend_ptrs.size() == 2);

            // find a transferor for this task
            const auto tx_ptr = m_transferor_registry.get(rinfo_ptrs[0]->type(), 
                                                          rinfo_ptrs[1]->type());
            if(!tx_ptr) {
                return std::make_tuple(urd_error::not_supported, boost::none);
            }

            LOGGER_DEBUG("Selected plugin: {}", tx_ptr->to_string());
            
            if(!tx_ptr->validate(rinfo_ptrs[0], rinfo_ptrs[1])) {
                return std::make_tuple(urd_error::bad_args, boost::none);
            }

            m_runners.submit_with_epilog_and_forget(
                io::task<iotask_type::copy>(
                    std::move(task_info_ptr), std::move(tx_ptr)), 
                register_completion);
            break;
        }

        case iotask_type::move:
        {
            assert(backend_ptrs.size() == 2);

            // find a transferor for this task
            const auto tx_ptr = m_transferor_registry.get(rinfo_ptrs[0]->type(), 
                                                          rinfo_ptrs[1]->type());
            if(!tx_ptr) {
                return std::make_tuple(urd_error::not_supported, boost::none);
            }

            LOGGER_DEBUG("Selected plugin: {}", tx_ptr->to_string());
            
            if(!tx_ptr->validate(rinfo_ptrs[0], rinfo_ptrs[1])) {
                return std::make_tuple(urd_error::bad_args, boost::none);
            }

            m_runners.submit_with_epilog_and_forget(
                io::task<iotask_type::move>(
                    std::move(task_info_ptr), std::move(tx_ptr)), 
                register_completion);
            break;
        }

        case iotask_type::noop:
        {
            assert(backend_ptrs.size() == 2);

            // find a transferor for this task
            const auto tx_ptr = m_transferor_registry.get(rinfo_ptrs[0]->type(), 
                                                          rinfo_ptrs[1]->type());
            if(!tx_ptr) {
                return std::make_tuple(urd_error::not_supported, boost::none);
            }

            LOGGER_DEBUG("Selected plugin: {}", tx_ptr->to_string());
            
            if(!tx_ptr->validate(rinfo_ptrs[0], rinfo_ptrs[1])) {
                return std::make_tuple(urd_error::bad_args, boost::none);
            }

            m_runners.submit_and_forget(
                io::task<iotask_type::noop>(std::move(task_info_ptr),
                                            m_dry_run_duration));
            break;
        }

        default:
            return std::make_tuple(urd_error::bad_args, boost::none);
    }

    return std::make_tuple(urd_error::success, tid);
}

std::tuple<urd_error, boost::optional<io::generic_task>>
task_manager::create_local_initiated_task(iotask_type type,
                            const auth::credentials& auth,
                            const std::vector<backend_ptr>& backend_ptrs,
                            const std::vector<resource_info_ptr>& rinfo_ptrs) {

    boost::unique_lock<boost::shared_mutex> lock(m_mutex);

    // fetch an iotask_id for this task
    iotask_id tid = ++m_id_base;

    if(m_task_info.count(tid) != 0) {
        --m_id_base;
        return std::make_tuple(urd_error::too_many_tasks, boost::none);
    }

    // immediately-invoked lambda to create the appropriate metadata for the task
    // and register it into m_task_info
    const auto task_info_ptr = [&]() {

        assert(backend_ptrs.size() == 1 || backend_ptrs.size() == 2);

        const backend_ptr src_backend = backend_ptrs[0];
        const resource_info_ptr src_rinfo = rinfo_ptrs[0];
        const backend_ptr dst_backend = (backend_ptrs.size() == 1 ? 
                                            nullptr : backend_ptrs[1]);
        const resource_info_ptr dst_rinfo = (rinfo_ptrs.size() == 1 ? 
                                            nullptr : rinfo_ptrs[1]);

        auto it = m_task_info.end();
        std::tie(it, std::ignore) = m_task_info.emplace(tid,
                std::make_shared<task_info>(tid, type, auth, 
                                            src_backend, src_rinfo,
                                            dst_backend, dst_rinfo));
        return it->second;
    }();


    std::shared_ptr<io::transferor> tx_ptr;

    if(type == iotask_type::copy || type == iotask_type::move) {

        assert(backend_ptrs.size() == 2);

        tx_ptr = m_transferor_registry.get(rinfo_ptrs[0]->type(), 
                                           rinfo_ptrs[1]->type());
        if(!tx_ptr) {
            return std::make_tuple(urd_error::not_supported, boost::none);
        }

        LOGGER_DEBUG("Selected plugin: {}", tx_ptr->to_string());

        if(!tx_ptr->validate(rinfo_ptrs[0], rinfo_ptrs[1])) {
            return std::make_tuple(urd_error::bad_args, boost::none);
        }
    }

    if(m_dry_run) {
        type = iotask_type::noop;
    }

    switch(type) {
        case iotask_type::remove:
        {
            assert(backend_ptrs.size() == 1);

            return std::make_tuple(
                    urd_error::success, 
                    generic_task(type,
                        io::task<iotask_type::remove>(
                            std::move(task_info_ptr))));
        }

        case iotask_type::copy:
        {
            return std::make_tuple(
                    urd_error::success,
                    generic_task(type,
                        io::task<iotask_type::copy>(
                            std::move(task_info_ptr), std::move(tx_ptr))));
            break;
        }

        case iotask_type::move:
        {

            return std::make_tuple(
                    urd_error::success,
                    generic_task(type,
                        io::task<iotask_type::move>(
                            std::move(task_info_ptr), std::move(tx_ptr))));
            break;
        }

        case iotask_type::noop:
        {
            return std::make_tuple(
                    urd_error::success,
                    generic_task(type,
                        io::task<iotask_type::noop>(
                            std::move(task_info_ptr), m_dry_run_duration)));
            break;
        }

        default:
            break;
    }

    return std::make_tuple(urd_error::bad_args, boost::none);
}

std::tuple<urd_error, boost::optional<io::generic_task>>
task_manager::create_remote_initiated_task(iotask_type task_type,
                                    const auth::credentials& auth,
                                    const boost::any& ctx,
                                    const backend_ptr src_backend,
                                    const resource_info_ptr src_rinfo,
                                    const backend_ptr dst_backend,
                                    const resource_info_ptr dst_rinfo) {

    boost::unique_lock<boost::shared_mutex> lock(m_mutex);

    // fetch an iotask_id for this task
    iotask_id tid = ++m_id_base;

    if(m_task_info.count(tid) != 0) {
        --m_id_base;
        return std::make_tuple(urd_error::too_many_tasks, boost::none);
    }

    // immediately-invoked lambda to create the appropriate metadata for the task
    // and register it into m_task_info
    const auto task_info_ptr = [&]() {

        auto it = m_task_info.end();
        std::tie(it, std::ignore) = m_task_info.emplace(tid,
                std::make_shared<task_info>(tid, task_type, auth, 
                                            src_backend, src_rinfo,
                                            dst_backend, dst_rinfo,
                                            ctx));
        return it->second;
    }();

    auto tx_ptr = 
        m_transferor_registry.get(src_rinfo->type(), dst_rinfo->type());

    if(!tx_ptr) {
        return std::make_tuple(urd_error::not_supported, boost::none);
    }

    LOGGER_DEBUG("Selected plugin: {}", tx_ptr->to_string());

    if(!tx_ptr->validate(src_rinfo, dst_rinfo)) {
        return std::make_tuple(urd_error::bad_args, boost::none);
    }

    return std::make_tuple(
            urd_error::success,
            generic_task(task_type,
                io::task<iotask_type::remote_transfer>(
                    std::move(task_info_ptr), std::move(tx_ptr))));
}


urd_error
task_manager::enqueue_task(io::generic_task&& t) {

    // helper lambda to register the completion of tasks so that we can keep track
    // of the consumed bandwidth by each task
    // N.B: we use capture-by-value here so that the task_info_ptr is valid when 
    // the callback is invoked.
    const auto completion_callback = [=]() {
        assert(t.info()->status() == task_status::finished ||
               t.info()->status() == task_status::finished_with_error);

        LOGGER_DEBUG("Task {} finished [{} MiB/s]", 
                t.info()->id(), t.info()->bandwidth());

        auto bw = t.info()->bandwidth();

        // bw might be nan if the task did not finish correctly
        if(!std::isnan(bw)) {

            const auto key = std::make_pair(t.info()->src_rinfo()->nsid(),
                                            t.info()->dst_rinfo()->nsid());

            if(!m_bandwidth_backlog.count(key)) {
                m_bandwidth_backlog.emplace(key, 
                        boost::circular_buffer<double>(m_backlog_size));
            }

            m_bandwidth_backlog.at(key).push_back(bw);
        }
    };

    switch(t.m_type) {
        case iotask_type::remove:
        case iotask_type::noop:
        {
            m_runners.submit_and_forget(t);
            break;
        }

        case iotask_type::copy:
        case iotask_type::move:
        {
            m_runners.submit_with_epilog_and_forget(t, completion_callback);
            break;
        }

        default:
            return urd_error::bad_args;
    }

    return urd_error::success;
}

std::shared_ptr<task_info>
task_manager::find(iotask_id tid) const {

    boost::shared_lock<boost::shared_mutex> lock(m_mutex);

    const auto& it = m_task_info.find(tid);

    if(it != m_task_info.end()) {
        return it->second;
    }

    return nullptr;
}

io::global_stats
task_manager::global_stats() const {

    uint32_t running_tasks = 0;
    uint32_t pending_tasks = 0;

    const auto get_avg_bandwidth = [&](const std::string& nsid1, 
                                       const std::string& nsid2) -> double {

        if(!m_bandwidth_backlog.count({nsid1, nsid2})) {
            return std::numeric_limits<double>::quiet_NaN();
        }

        if(m_bandwidth_backlog.at({nsid1, nsid2}).size() == 0) {
            return std::numeric_limits<double>::quiet_NaN();
        }

        return std::accumulate(m_bandwidth_backlog.at({nsid1, nsid2}).begin(),
                               m_bandwidth_backlog.at({nsid1, nsid2}).end(),
                               0.0) / m_bandwidth_backlog.size();
    };

    std::vector<double> etas;

    // lock map so that no new tasks are added until we finish
    // (though the stats for already existing tasks can still be updated)
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);

    bool unreliable_eta = false;

    for(const auto& kv : m_task_info) {

        // stats() locks the task_info before creating the result, so that 
        // the data retrieved is stable
        const task_stats st{kv.second->stats()};

        switch(st.status()) {
            case task_status::running:
            {
                ++running_tasks;

                const std::string& nsid1 = kv.second->src_rinfo()->nsid();
                const std::string& nsid2 = kv.second->dst_rinfo()->nsid();
                const double avg_bw = get_avg_bandwidth(nsid1, nsid2);

                // if avg_bw is NAN, we can't estimate the ETA reliably
                if(std::isnan(avg_bw)) {
                    unreliable_eta = true;
                }

                etas.push_back(static_cast<double>(
                            st.pending_bytes()/(1024*1024))/avg_bw);

                LOGGER_DEBUG("Pending {} bytes for task {}: E.T.A {} seconds "
                             "(inter-namespace avg bw: {} MiB/s)", 
                             st.pending_bytes(), kv.first, etas.back(), avg_bw);
                break;
            }

            case task_status::pending:
                ++pending_tasks;
                break;

            default:
                break;
        }
    }

    const double eta = [&]() -> double {
        if(running_tasks == 0) {
            return 0.0;
        }

        if(unreliable_eta) {
            return std::numeric_limits<double>::quiet_NaN();
        }

        return *std::max_element(etas.begin(), etas.end());
    }();

    LOGGER_DEBUG("E.T.A. for all running tasks: {} seconds", eta);

    return io::global_stats(running_tasks, pending_tasks, eta);
}

void
task_manager::stop_all_tasks() {
    m_runners.stop();
}

} // namespace io
} // namespace norns
