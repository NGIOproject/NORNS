/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
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
#include <boost/filesystem/fstream.hpp>
#include <boost/system/system_error.hpp>
#include "backend-base.hpp"
#include "resources.hpp"
#include "posix-fs.hpp"
#include "utils.hpp"
#include "logger.hpp"

namespace bfs = boost::filesystem;

namespace {

bool contains(const bfs::path& p1, const bfs::path& p2) {

    for(const auto& c : p1) {
        if(c == p2) {
            return true;
        }
    }

    return false;
}

}

namespace norns {
namespace storage {

posix_filesystem::posix_filesystem(const bfs::path& mount, uint32_t quota) 
    : m_mount(mount), m_quota(quota) { }

bfs::path posix_filesystem::mount() const {
    return m_mount;
}

uint32_t posix_filesystem::quota() const {
    return m_quota;
}

backend::resource_ptr 
posix_filesystem::new_resource(const resource_info_ptr& rinfo, 
                               bool is_collection, std::error_code& ec) const {

    const auto d_rinfo = std::static_pointer_cast<data::local_path_info>(rinfo);

    const bfs::path ns_subpath = utils::lexical_normalize(d_rinfo->datapath(), is_collection);

    if(ns_subpath.empty()) {
        ec = std::make_error_code(static_cast<std::errc>(ENOENT));
        return backend::resource_ptr();
    }

    const bfs::path parents = ns_subpath.parent_path();

    if(!parents.empty() && parents != "/") {
        boost::system::error_code error;
        create_directories(m_mount / parents, error);

        if(error) {
            ec = std::make_error_code(static_cast<std::errc>(error.value()));
            return backend::resource_ptr();
        }
    }

    return std::make_shared<data::local_path_resource>(shared_from_this(), ns_subpath);
}

backend::resource_ptr posix_filesystem::get_resource(const resource_info_ptr& rinfo,
                                                     std::error_code& ec) const {

    const auto d_rinfo = std::static_pointer_cast<data::local_path_info>(rinfo);

    const bfs::path ns_subpath = utils::lexical_normalize(d_rinfo->datapath(), false);

    if(ns_subpath.empty()) {
        ec = std::make_error_code(static_cast<std::errc>(ENOENT));
        return backend::resource_ptr();
    }

    // check that path exists
    boost::system::error_code error;
    const bfs::path canonical_path = bfs::canonical(m_mount / ns_subpath, error);

    if(error) {
        ec = std::make_error_code(static_cast<std::errc>(error.value()));
        return backend::resource_ptr();
    }

    // path exists: compute the absolute subpath considering the backend mount as root
    const bfs::path ns_abs_subpath = bfs::path("/") / 
                                bfs::relative(m_mount, canonical_path) /
                                (bfs::is_directory(canonical_path) ? "/" : "");

    // if the computed subpath is relative, it means that the canonical_path
    // that we got in the previous step is not contained by m_mount.
    // This might happen if we followed a soft link pointing to a valid path
    // located outside the namespace. Since we don't want to allow this, we
    // return ENOENT, so as to not give a clue whether the dst path exists or not
    if(::contains(ns_abs_subpath, "..")) {
        ec = std::make_error_code(static_cast<std::errc>(ENOENT));
        return backend::resource_ptr();
    }

    ec = std::make_error_code(static_cast<std::errc>(0));
    return std::make_shared<data::local_path_resource>(shared_from_this(), ns_abs_subpath);
}

std::size_t
posix_filesystem::get_size(const resource_info_ptr& rinfo, std::error_code& ec) const {

    const auto d_rinfo = std::static_pointer_cast<data::local_path_info>(rinfo);

    const bfs::path ns_subpath = utils::lexical_normalize(d_rinfo->datapath(), false);

    if(ns_subpath.empty()) {
        ec = std::make_error_code(static_cast<std::errc>(ENOENT));
        return 0;
    }

    // check that path exists
    boost::system::error_code error;
    const bfs::path canonical_path = bfs::canonical(m_mount / ns_subpath, error);

    if(error) {
        ec = std::make_error_code(static_cast<std::errc>(error.value()));
        return 0;
    }

    const std::size_t sz = [&]() -> std::size_t {
        std::size_t bytes = 0;
        if(bfs::is_directory(canonical_path)) {
            for(bfs::recursive_directory_iterator it(canonical_path, error);
                it != bfs::recursive_directory_iterator();
                ++it) {

                if(error) {
                    return 0;
                }

                if(!bfs::is_regular(*it)) {
                    continue;
                }

                bytes += bfs::file_size(*it);
            }

            return bytes;
        }

        return bfs::file_size(canonical_path, error);
    }();

    if(error) {
        ec = std::make_error_code(static_cast<std::errc>(error.value()));
        return 0;
    }

    return sz;
}

bool posix_filesystem::accepts(resource_info_ptr res) const {
    switch(res->type()) {
        case data::resource_type::local_posix_path:
            return true;
        default:
            return false;
    }
}

std::string posix_filesystem::to_string() const {
    return "POSIX_FILESYSTEM(" + m_mount.string() + ", " + std::to_string(m_quota) + ")";
}

} // namespace storage
} // namespace norns
