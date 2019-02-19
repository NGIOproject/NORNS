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

#include <boost/filesystem.hpp>
#include <system_error>
#include <libtar.h>

namespace bfs = boost::filesystem;

namespace norns {
namespace utils {

struct tar {

    enum class openmode : int {
        create = 0,
        open   = 1,
    };

    constexpr static openmode create = openmode::create;
    constexpr static openmode open = openmode::open;

    tar(const bfs::path& filename, openmode op, std::error_code& ec);

    void
    add_directory(const bfs::path& real_dir, 
                  const bfs::path& archive_dir,
                  std::error_code& ec);

    void
    extract(const bfs::path& parent_dir,
            std::error_code& ec);

    bfs::path
    path() const;

    ~tar();

    TAR* m_tar = nullptr;
    bfs::path m_path;
};

} // namespace utils
} // namespace norns
