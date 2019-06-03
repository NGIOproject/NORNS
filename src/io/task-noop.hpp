#ifndef __IO_TASK_NOOP_HPP__
#define __IO_TASK_NOOP_HPP__

namespace norns {
namespace io {

/////////////////////////////////////////////////////////////////////////////////
//   specializations for noop tasks 
/////////////////////////////////////////////////////////////////////////////////
template <>
struct task<iotask_type::noop> {

    using task_info_ptr = std::shared_ptr<task_info>;

    task(const task_info_ptr&& task_info, uint32_t sleep_duration)
        : m_task_info(std::move(task_info)),
          m_sleep_duration(sleep_duration) { }

    task(const task& other) = default;
    task(task&& rhs) = default;
    task& operator=(const task& other) = default;
    task& operator=(task&& rhs) = default;

    void operator()() {
        const auto tid = m_task_info->id();

        LOGGER_WARN("[{}] Starting noop I/O task", tid);

        LOGGER_DEBUG("[{}] Sleep for {} usecs", tid, m_sleep_duration);
        usleep(m_sleep_duration);

        m_task_info->update_status(task_status::running);

        LOGGER_WARN("[{}] noop I/O task \"running\"", tid);

        LOGGER_DEBUG("[{}] Sleep for {} usecs", tid, m_sleep_duration);
        usleep(m_sleep_duration);

        m_task_info->update_status(task_status::finished);

        LOGGER_WARN("[{}] noop I/O task completed successfully", tid);
    }

    task_info_ptr m_task_info;
    uint32_t m_sleep_duration;
};

} // namespace io
} // namespace norns

#endif // __IO_TASK_NOOP_HPP__
