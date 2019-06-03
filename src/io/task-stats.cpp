#include "common.hpp"
#include "task-stats.hpp"

namespace norns {
namespace io {

task_stats::task_stats() :
    m_status(task_status::undefined),
    m_task_error(urd_error::success),
    m_sys_error(),
    m_total_bytes(),
    m_pending_bytes() { }

task_stats::task_stats(task_status st, urd_error ec, const std::error_code& sc,
            std::size_t total_bytes, std::size_t pending_bytes) :
    m_status(st),
    m_task_error(ec),
    m_sys_error(sc),
    m_total_bytes(total_bytes),
    m_pending_bytes(pending_bytes) { }

std::size_t task_stats::pending_bytes() const {
    return m_pending_bytes;
}

std::size_t task_stats::total_bytes() const {
    return m_total_bytes;
}

task_status task_stats::status() const {
    return m_status;
}

void task_stats::set_status(const task_status status) {
    m_status = status;
}


urd_error task_stats::error() const {
    return m_task_error;
}

void task_stats::set_error(const urd_error ec) {
    m_task_error = ec;
}

std::error_code task_stats::sys_error() const {
    return m_sys_error;
}

void task_stats::set_sys_error(const std::error_code& ec) {
    m_sys_error = ec;
}

global_stats::global_stats() :
    m_running_tasks(0),
    m_pending_tasks(0),
    m_eta(std::numeric_limits<double>::quiet_NaN()) {}

global_stats::global_stats(uint32_t running_tasks, uint32_t pending_tasks, 
                           double eta) :
    m_running_tasks(running_tasks),
    m_pending_tasks(pending_tasks),
    m_eta(eta) {}

uint32_t
global_stats::running_tasks() const {
    return m_running_tasks;
}

uint32_t
global_stats::pending_tasks() const {
    return m_pending_tasks;
}

double
global_stats::eta() const {
    return m_eta;
}

} // namespace io

namespace utils {

std::string to_string(io::task_status st) {
    switch(st) {
        case io::task_status::pending:
            return "NORNS_EPENDING";
        case io::task_status::running:
            return "NORNS_EINPROGRESS";
        case io::task_status::finished:
            return "NORNS_EFINISHED";
        case io::task_status::finished_with_error:
            return "NORNS_EFINISHEDWERROR";
        default:
            return "unknown!";
    }
}

std::string to_string(const io::global_stats& gst) {
    return "(r: " + std::to_string(gst.running_tasks()) + 
           ", p:" + std::to_string(gst.pending_tasks()) + 
           ", eta: " + std::to_string(gst.eta()) + ")";
}


} // namespace utils
} // namespace norns
