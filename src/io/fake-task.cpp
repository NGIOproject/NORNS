#include <unistd.h>
#include <system_error>

#include "logger.hpp"
#include "fake-task.hpp"

namespace norns {
namespace io {

fake_task::fake_task(task_info_ptr&& task_info)
    : task(std::move(task_info), nullptr) {}

void fake_task::operator()() {
    LOGGER_WARN("[{}] Starting fake I/O task", m_id);

    usleep(100);

    m_task_info->update_status(task_status::in_progress);

    LOGGER_WARN("[{}] fake I/O task \"running\"", m_id);

    usleep(100);

    m_task_info->update_status(task_status::finished);

    LOGGER_WARN("[{}] fake I/O task completed successfully", m_id);
}

} // namespace io
} // namespace norns


