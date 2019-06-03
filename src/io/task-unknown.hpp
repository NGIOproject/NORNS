#ifndef __IO_TASK_UNKNOWN_HPP__
#define __IO_TASK_UNKNOWN_HPP__

namespace norns {
namespace io {

/////////////////////////////////////////////////////////////////////////////////
//   specializations for unknown tasks 
/////////////////////////////////////////////////////////////////////////////////
template <>
struct task<iotask_type::unknown> {
    task() { }
    task(const task& other) = default;
    task(task&& rhs) = default;
    task& operator=(const task& other) = default;
    task& operator=(task&& rhs) = default;
    void operator()() { }
};

} // namespace io
} // namespace norns

#endif // __IO_TASK_UNKNOWN_HPP__
