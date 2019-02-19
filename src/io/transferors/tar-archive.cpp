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

#include "utils.hpp"
#include "logger.hpp"
#include "tar-archive.hpp"

namespace norns {
namespace utils {

tar::tar(const bfs::path& filename, 
         openmode op,
         std::error_code& ec) :
    m_path(filename) {

    constexpr const std::array<int, 2> flags = {
        {O_WRONLY | O_CREAT| O_EXCL, O_RDONLY}
    };

    constexpr const std::array<int, 2> modes = {
        {S_IRUSR | S_IWUSR, 0}
    };

    if(tar_open(&m_tar, m_path.c_str(), NULL, 
                flags[static_cast<int>(op)], 
                modes[static_cast<int>(op)], TAR_GNU) != 0) {
        ec = std::make_error_code(static_cast<std::errc>(errno)); 
        LOGGER_ERROR("Failed to open archive for writing: {}", 
                     logger::errno_message(ec.value()));
        return;
    }
}

void
tar::add_directory(const bfs::path& real_dir, 
                   const bfs::path& archive_dir,
                   std::error_code& ec) {

    const bfs::path rd = 
        norns::utils::remove_trailing_separator(real_dir);
    const bfs::path ad = 
        norns::utils::remove_trailing_separator(archive_dir);

    if(tar_append_tree(m_tar, const_cast<char*>(rd.c_str()),
                        const_cast<char*>(ad.c_str())) != 0) {
        ec = std::make_error_code(static_cast<std::errc>(errno));
        return;
    }

    ec = std::make_error_code(static_cast<std::errc>(0)); 
}

void
tar::extract(const bfs::path& parent_dir, 
             std::error_code& ec) {

    if(m_tar == nullptr) {
        ec = std::make_error_code(static_cast<std::errc>(EINVAL)); 
        return;
    }

    if(tar_extract_all(m_tar, const_cast<char*>(parent_dir.c_str())) != 0) {
        ec = std::make_error_code(static_cast<std::errc>(errno));
    }
}


bfs::path
tar::path() const {
    return m_path;
}

tar::~tar() {

    if(m_tar != nullptr) {
        if(tar_close(m_tar) != 0) {
            LOGGER_ERROR("Failed to close TAR archive: {}",
                        logger::errno_message(errno));
        }
    }
}

} // namespace utils
} // namespace norns
