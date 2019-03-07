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

#ifndef __TASK_MANAGER_HPP__
#define __TASK_MANAGER_HPP__

#include <memory>
#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/circular_buffer.hpp>
#include "thread-pool.hpp"
#include "task.hpp"
#include "common.hpp"

namespace norns {

// forward declarations
namespace data {
enum class resource_type;
}

namespace io {

// forward declarations
enum class task_status;
struct task_stats;
struct task_info;

struct task_manager {

    struct pair_hash {
        template <typename T, typename U>
        std::size_t operator()(const std::pair<T, U> &x) const {
            std::size_t seed = 0;
            boost::hash_combine(seed, x.first);
            boost::hash_combine(seed, x.second);
            return seed;
        }
    };

    using key_type = iotask_id; 
    using value_type = std::shared_ptr<task_info>;
    using backend_ptr = std::shared_ptr<storage::backend>;
    using resource_info_ptr = std::shared_ptr<data::resource_info>;
    using resource_ptr = std::shared_ptr<data::resource>;
    using transferor_ptr = std::shared_ptr<transferor>;
    using ReturnType = std::tuple<iotask_id, std::shared_ptr<task_info>>;

    task_manager(uint32_t nrunners, 
                 uint32_t backlog_size, 
                 bool dry_run, 
                 uint32_t dry_run_duration);

    bool
    register_transfer_plugin(const data::resource_type t1,
                             const data::resource_type t2,
                             std::shared_ptr<io::transferor>&& trp);

    boost::optional<iotask_id>
    create_task(iotask_type type, const auth::credentials& creds, 
            const backend_ptr src_backend, const resource_info_ptr src_rinfo, 
            const backend_ptr dst_backend, const resource_info_ptr dst_rinfo,
            const transferor_ptr&& tx_ptr);

    std::tuple<urd_error, boost::optional<iotask_id>>
    create_task(iotask_type type, const auth::credentials& auth,
                const std::vector<std::shared_ptr<storage::backend>>& backend_ptrs,
                const std::vector<std::shared_ptr<data::resource_info>>& rinfo_ptrs);

    std::tuple<urd_error, boost::optional<io::generic_task>>
    create_local_initiated_task(iotask_type type,
                        const auth::credentials& auth,
                        const std::vector<backend_ptr>& backend_ptrs,
                        const std::vector<resource_info_ptr>& rinfo_ptrs);

    std::tuple<urd_error, boost::optional<io::generic_task>>
    create_remote_initiated_task(iotask_type task_type,
                        const auth::credentials& auth,
                        const boost::any& ctx,
                        const backend_ptr src_backend,
                        const resource_info_ptr src_rinfo,
                        const backend_ptr dst_backend,
                        const resource_info_ptr dst_rinfo);

    urd_error
    enqueue_task(io::generic_task&& t);

    std::shared_ptr<task_info>
    find(iotask_id) const;

    template <typename UnaryPredicate>
    std::size_t
    count_if(UnaryPredicate&& p) {
        boost::unique_lock<boost::shared_mutex> lock(m_mutex);
        return std::count_if(m_task_info.begin(),
                             m_task_info.end(),
                             [&](const std::pair<key_type, value_type>& kv) {
                                return p(kv.second);
                             });
    }

    io::global_stats
    global_stats() const;

    void 
    stop_all_tasks();

private:
    mutable boost::shared_mutex m_mutex;
    iotask_id m_id_base = 0;
    const uint32_t m_backlog_size;
    bool m_dry_run;
    uint32_t m_dry_run_duration;
    std::unordered_map<iotask_id, std::shared_ptr<task_info>> m_task_info;
    std::unordered_map<std::pair<std::string, std::string>,
                       boost::circular_buffer<double>, pair_hash> m_bandwidth_backlog;
    thread_pool m_runners;
    io::transferor_registry m_transferor_registry;
};

} // namespace io
} // namespace norns

#endif /* __TASK_MANAGER_HPP__ */
