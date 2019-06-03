#include "logger.hpp"

namespace norns {
namespace utils {

struct file_handle {

    constexpr static const int init_value{-1};

    file_handle() = default;

    explicit file_handle(int fd) noexcept :
        m_fd(fd) { }

    file_handle(file_handle&& rhs) = default;
    file_handle(const file_handle& other) = delete;
    file_handle& operator=(file_handle&& rhs) = default;
    file_handle& operator=(const file_handle& other) = delete;

    explicit operator bool() const noexcept {
        return valid();
    }

    bool operator!() const noexcept {
        return !valid();
    }

    bool
    valid() const noexcept {
        return m_fd != init_value;
    }

    int
    native() const noexcept {
        return m_fd;
    }

    int
    release() noexcept {
        int ret = m_fd;
        m_fd = init_value;
        return ret;
    }

    ~file_handle() {
        if(m_fd != init_value) {
            if(::close(m_fd) == -1) {
                LOGGER_ERROR("Failed to close file descriptor: {}",
                             logger::errno_message(errno));
            }
        }
    }

    int m_fd;
};



} // namespace utils
} // namespace norns
