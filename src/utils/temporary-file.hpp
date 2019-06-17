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

#ifndef NORNS_UTILS_TEMP_FILE_HPP
#define NORNS_UTILS_TEMP_FILE_HPP

#include <boost/filesystem.hpp>
#include <system_error>

namespace bfs = boost::filesystem;

namespace norns {
namespace utils {

struct temporary_file {

    temporary_file() noexcept;

    // create an empty temporary file from pattern at parent_dir
    temporary_file(const std::string& pattern,
                   const bfs::path& parent_dir,
                   std::error_code& ec) noexcept;

    // create a temporary file of size 'prealloc_size' from pattern at parent_dir
    temporary_file(const std::string& pattern,
                   const bfs::path& parent_dir,
                   std::size_t prealloc_size,
                   std::error_code& ec) noexcept;

    // initialize temporary file from an already existing file
    temporary_file(const bfs::path& filename,
                   std::error_code& ec) noexcept;

    temporary_file(const temporary_file& other) = delete;

    temporary_file(temporary_file&& rhs) = default;

    temporary_file& operator=(const temporary_file& other) = delete;

    temporary_file& operator=(temporary_file&& rhs) = default;

    ~temporary_file();

    bfs::path 
    path() const noexcept;

    void
    reserve(std::size_t size, 
            std::error_code& ec) const noexcept;

    void
    manage(const bfs::path& filename,
           std::error_code& ec) noexcept;

    bfs::path
    release() noexcept;

    bfs::path m_filename;
};

} // namespace utils
} // namespace norns

#endif // NORNS_UTILS_TEMP_FILE_HPP
