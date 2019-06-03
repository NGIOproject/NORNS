#ifndef __TASK_INFO_HPP__
#define __TASK_INFO_HPP__

#include <boost/any.hpp>
#include <boost/thread/shared_mutex.hpp>
#include "backends.hpp"
#include "resources.hpp"
#include "auth.hpp"

namespace norns {
namespace io {

// forward declaration
enum class task_status;
struct task_stats;

struct task_info {

    using backend_ptr = std::shared_ptr<storage::backend>;
    using resource_info_ptr = std::shared_ptr<data::resource_info>;

    task_info(const iotask_id tid, 
              const iotask_type type, 
              const bool is_remote,
              const auth::credentials& creds,
              const backend_ptr src_backend, 
              const resource_info_ptr src_rinfo,
              const backend_ptr dst_backend, 
              const resource_info_ptr dst_rinfo,
              const boost::any& ctx = {});

    ~task_info();

    iotask_id 
    id() const;

    iotask_type 
    type() const;

    bool 
    is_remote() const;

    auth::credentials 
    auth() const ;

    backend_ptr 
    src_backend() const;

    resource_info_ptr 
    src_rinfo() const;

    backend_ptr 
    dst_backend() const;

    resource_info_ptr 
    dst_rinfo() const;

    boost::any 
    context() const;

    void 
    set_context(const boost::any& ctx);

    void 
    clear_context();

    task_status 
    status() const;

    void 
    update_status(const task_status st);

    void 
    update_status(const task_status st, const urd_error ec,
                  const std::error_code& sc);

    urd_error 
    task_error() const;

    std::error_code 
    sys_error() const;

    std::size_t 
    sent_bytes() const;

    std::size_t 
    total_bytes() const;

    double 
    bandwidth() const;

    void 
    update_bandwidth(std::size_t bytes, double usecs);

    void 
    record_transfer(std::size_t bytes, double usecs);

    task_stats 
    stats() const;

    boost::shared_lock<boost::shared_mutex>
    lock_shared() const;

    boost::unique_lock<boost::shared_mutex>
    lock_unique() const;

    mutable boost::shared_mutex m_mutex;

    // task id and type
    const iotask_id m_id;
    const iotask_type m_type;
    const bool m_is_remote;

    // user credentials
    const auth::credentials m_auth;

    // backends and resources involved in task
    const backend_ptr m_src_backend;
    const resource_info_ptr m_src_rinfo;
    const backend_ptr m_dst_backend;
    const resource_info_ptr m_dst_rinfo;

    // optional task context
    boost::any m_ctx;

    // general task status
    task_status m_status;
    urd_error m_task_error;
    std::error_code m_sys_error;

    // some statistics
    double m_bandwidth;
    std::size_t m_sent_bytes;
    std::size_t m_total_bytes;
};

} // namespace io
} // namespace norns
#endif /* __TASK_INFO_HPP__ */
