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

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/system/system_error.hpp>
#include "backend-base.hpp"
#include "resources.hpp"
#include "posix-fs.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include <iostream>

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

posix_filesystem::posix_filesystem(const std::string& nsid, 
                                   bool track, 
                                   const bfs::path& mount, 
                                   uint32_t quota)
    : m_nsid(nsid),
      m_track(track),
      m_mount(mount), 
      m_quota(quota) { }

std::string
posix_filesystem::nsid() const {
    return m_nsid;
}

bool
posix_filesystem::is_tracked() const {
    return m_track;
}

bool
posix_filesystem::is_empty() const {
    return (bfs::recursive_directory_iterator(m_mount) == 
            bfs::recursive_directory_iterator());
}

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
                                bfs::relative(canonical_path, m_mount) /
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

void
posix_filesystem::remove(const resource_info_ptr& rinfo, std::error_code& ec) const {

    const auto d_rinfo = std::static_pointer_cast<data::local_path_info>(rinfo);
    const bfs::path ns_subpath = utils::lexical_normalize(d_rinfo->datapath(), false);

    if(ns_subpath.empty()) {
        ec = std::make_error_code(static_cast<std::errc>(ENOENT));
        return;
    }

    // check that path exists
    boost::system::error_code error;
    const bfs::path canonical_path = [&]() {

        const bfs::path p{m_mount / ns_subpath};

        if(!bfs::is_symlink(p)) {
            return bfs::canonical(p, error);
        }

        bfs::exists(p, error);
        return p;
    }();
        
    if(error) {
        ec = std::make_error_code(static_cast<std::errc>(error.value()));
        return;
    }

    bfs::remove_all(canonical_path, error);

    if(error) {
        ec = std::make_error_code(static_cast<std::errc>(error.value()));
        return;
    }
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
