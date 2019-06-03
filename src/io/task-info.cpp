#include <boost/version.hpp>
#include <cmath>
#include <limits>
#include "task-stats.hpp"
#include "task-info.hpp"
#include "logger.hpp"

namespace norns {
namespace io {

task_info::task_info(const iotask_id tid, 
                     const iotask_type type,
                     const bool is_remote,
                     const auth::credentials& auth,
                     const backend_ptr src_backend, 
                     const resource_info_ptr src_rinfo,
                     const backend_ptr dst_backend, 
                     const resource_info_ptr dst_rinfo,
                     const boost::any& ctx) :
    m_id(tid),
    m_type(type),
    m_is_remote(is_remote),
    m_auth(auth),
    m_src_backend(src_backend),
    m_src_rinfo(src_rinfo),
    m_dst_backend(dst_backend),
    m_dst_rinfo(dst_rinfo),
    m_ctx(ctx),
    m_status(task_status::pending),
    m_task_error(urd_error::success),
    m_sys_error(),
    m_bandwidth(std::numeric_limits<double>::quiet_NaN()),
    m_sent_bytes(),
    m_total_bytes() {

    if(!src_rinfo) {
        return;
    }

    std::error_code ec;
    m_total_bytes = src_backend->get_size(src_rinfo, ec);

    // make sure that m_total_bytes is 0 if an error occurred
    if(ec) {
        m_total_bytes = 0;
    }
}

task_info::~task_info() { }

iotask_id
task_info::id() const {
    return m_id;
}

iotask_type
task_info::type() const {
    return m_type;
}

bool 
task_info::is_remote() const {
    return m_is_remote;
}

auth::credentials 
task_info::auth() const {
    return m_auth;
}

task_info::backend_ptr 
task_info::src_backend() const {
    return m_src_backend;
}

task_info::resource_info_ptr 
task_info::src_rinfo() const {
    return m_src_rinfo;
}

task_info::backend_ptr 
task_info::dst_backend() const {
    return m_dst_backend;
}

task_info::resource_info_ptr 
task_info::dst_rinfo() const {
    return m_dst_rinfo;
}

boost::any
task_info::context() const {
    return m_ctx;
}

void 
task_info::set_context(const boost::any& ctx) {
    m_ctx = ctx;
}

void
task_info::clear_context() {
#if BOOST_VERSION <= 105500
    m_ctx = boost::any();
#else
    m_ctx.clear();
#endif
}

task_status
task_info::status() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return m_status;
}

void
task_info::update_status(const task_status st) {
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    m_status = st;
}

void 
task_info::update_status(const task_status st, const urd_error ec, 
                         const std::error_code& sc) {

    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    m_status = st;
    m_task_error = ec;
    m_sys_error = sc;
}

urd_error 
task_info::task_error() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return m_task_error;
}

std::error_code 
task_info::sys_error() const {
    return m_sys_error;
}

std::size_t
task_info::sent_bytes() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return m_sent_bytes;
}

std::size_t
task_info::total_bytes() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return m_total_bytes;
}

task_stats 
task_info::stats() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return task_stats(m_status, m_task_error, m_sys_error,
                      m_total_bytes, m_total_bytes - m_sent_bytes);
}

double
task_info::bandwidth() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return m_bandwidth;
}

void
task_info::update_bandwidth(std::size_t bytes, double usecs) {
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    m_bandwidth = (static_cast<double>(bytes)/(1024*1024) / (usecs/1e6));

    LOGGER_DEBUG("[{}] {}({}, {}) => {}", m_id, __FUNCTION__, bytes, usecs, m_bandwidth);
}

void
task_info::record_transfer(std::size_t bytes, double usecs) {
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    m_sent_bytes += bytes;
    m_bandwidth = (static_cast<double>(bytes)/(1024*1024) / (usecs/1e6));

    LOGGER_DEBUG("[{}] {}({}, {}) => {}", m_id, __FUNCTION__, bytes, usecs, m_bandwidth);
}

boost::shared_lock<boost::shared_mutex>
task_info::lock_shared() const {
    boost::shared_lock<boost::shared_mutex> lock(m_mutex);
    return lock;
}

boost::unique_lock<boost::shared_mutex>
task_info::lock_unique() const {
    boost::unique_lock<boost::shared_mutex> lock(m_mutex);
    return lock;
}

} // namespace io
} // namespace norns
