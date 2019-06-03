#ifndef __IO_TASK_STATS_HPP__
#define __IO_TASK_STATS_HPP__

#include <string>
#include <system_error>
#include <limits>

namespace norns {

enum class urd_error;

namespace io {

/*! Valid status for an I/O task */
enum class task_status {
    undefined,
    pending,
    running,
    finished,
    finished_with_error,
};

/*! Stats about a registered I/O task */
struct task_stats {

    task_stats();
    task_stats(task_status st, urd_error ec, const std::error_code& sc, 
               std::size_t total_bytes, std::size_t pending_bytes);

    std::size_t pending_bytes() const;
    std::size_t total_bytes() const;
    task_status status() const;
    void set_status(const task_status status);
    urd_error error() const;
    void set_error(const urd_error ec);
    std::error_code sys_error() const;
    void set_sys_error(const std::error_code& ec);

    task_status m_status;
    urd_error m_task_error;
    std::error_code m_sys_error;
    std::size_t m_total_bytes;
    std::size_t m_pending_bytes;
};

/*! Global stats about all registered I/O tasks */
struct global_stats {
    global_stats();
    global_stats(uint32_t running_tasks, uint32_t pending_tasks, double eta);

    uint32_t running_tasks() const;
    uint32_t pending_tasks() const;
    double eta() const;

    uint32_t m_running_tasks;
    uint32_t m_pending_tasks;
    double m_eta;
};

} // namespace io

namespace utils {

std::string to_string(io::task_status st);
std::string to_string(const io::global_stats& gst);

}

} // namespace norns

#endif /* __IO_TASK_STATS_HPP__ */
