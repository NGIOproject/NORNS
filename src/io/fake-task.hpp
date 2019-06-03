#ifndef __IO_FAKE_TASK_HPP__
#define __IO_FAKE_TASK_HPP__

#include <cstdint>
#include <memory>

#include "common.hpp"
#include "resources.hpp"
#include "backends.hpp"
#include "task.hpp"
#include "task-info.hpp"

namespace norns {
namespace io {

// forward declaration
struct task_stats;

/*! Descriptor for a fake I/O task that does nothing but waiting */
struct fake_task : public task {
    fake_task(task_info_ptr&& task_info);
    void operator()() override final;
};


} // namespace io
} // namespace norns

#endif // __IO_FAKE_TASK_HPP__
