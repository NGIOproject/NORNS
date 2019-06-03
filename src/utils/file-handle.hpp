/************************************************************************* 
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of NORNS, a service that allows other programs to   *
 * start, track and manage asynchronous transfers of data resources      *
 * between different storage backends.                                   *
 *                                                                       *
 * See AUTHORS file in the top level directory for information regarding *
 * developers and contributors.                                          *
 *                                                                       *
 * This software was developed as part of the EC H2020 funded project    *
 * NEXTGenIO (Project ID: 671951).                                       *
 *     www.nextgenio.eu                                                  *
 *                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining *
 * a copy of this software and associated documentation files (the       *
 * "Software"), to deal in the Software without restriction, including   *
 * without limitation the rights to use, copy, modify, merge, publish,   *
 * distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to *
 * the following conditions:                                             *
 *                                                                       *
 * The above copyright notice and this permission notice shall be        *
 * included in all copies or substantial portions of the Software.       *
 *                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                 *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS   *
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN    *
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN     *
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE      *
 * SOFTWARE.                                                             *
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
