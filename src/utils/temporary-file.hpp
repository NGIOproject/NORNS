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
