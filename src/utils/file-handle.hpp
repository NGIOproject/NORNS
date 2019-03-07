/*************************************************************************
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
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
