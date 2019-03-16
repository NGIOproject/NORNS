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

#ifndef NORNS_UTILS_TAR_ARCHIVE_HPP
#define NORNS_UTILS_TAR_ARCHIVE_HPP

#include <boost/filesystem.hpp>
#include <system_error>

// forward declare 'struct archive'
struct archive;

namespace bfs = boost::filesystem;

namespace norns {
namespace utils {

struct tar {

    enum class openmode : int {
        create = 0,
        open   = 1,
    };

    constexpr static const auto TAR_BLOCK_SIZE = 512;
    constexpr static const openmode create = openmode::create;
    constexpr static const openmode open = openmode::open;

    tar(const bfs::path& filename, openmode op, std::error_code& ec);

    ~tar();

    void
    add_file(const bfs::path& real_name, 
             const bfs::path& archive_name,
             std::error_code& ec);

    void
    add_directory(const bfs::path& real_dir, 
                  const bfs::path& archive_dir,
                  std::error_code& ec);

    void
    release();

    void
    extract(const bfs::path& parent_dir,
            std::error_code& ec);

    bfs::path
    path() const;

    static std::size_t
    estimate_size_once_packed(const bfs::path& source_path,
                              /*const bfs::path& packed_path,*/
                              std::error_code& ec);

    struct archive* m_archive = nullptr;
    bfs::path m_path;
    openmode m_openmode;
};

} // namespace utils
} // namespace norns

#endif // NORNS_UTILS_TAR_ARCHIVE_HPP
