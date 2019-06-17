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

#include "backend-base.hpp"
#include "resources.hpp"
#include "utils.hpp"
#include "nvml-dax.hpp"

namespace norns {
namespace storage {

nvml_dax::nvml_dax(const std::string& nsid, bool track, const bfs::path& mount, uint32_t quota) 
    : m_nsid(nsid),
      m_track(track),
      m_mount(mount),
      m_quota(quota) { }

std::string 
nvml_dax::nsid() const {
    return m_nsid;
}

bool
nvml_dax::is_tracked() const {
    return m_track;
}

bool
nvml_dax::is_empty() const {
    return false;
}

bfs::path nvml_dax::mount() const {
    return m_mount;
}

uint32_t nvml_dax::quota() const {
    return m_quota;
}

backend::resource_ptr 
nvml_dax::new_resource(const resource_info_ptr& rinfo, 
                       const bool is_collection, 
                       std::error_code& ec) const {
    (void) rinfo;
    (void) is_collection;
    (void) ec;
    return std::make_shared<data::local_path_resource>(shared_from_this(), ""); //XXX
}

backend::resource_ptr
nvml_dax::get_resource(const resource_info_ptr& rinfo, std::error_code& ec) const {
    (void) rinfo;
    (void) ec;
    return std::make_shared<data::local_path_resource>(shared_from_this(), ""); //XXX
}

void
nvml_dax::remove(const resource_info_ptr& rinfo, std::error_code& ec) const {
    (void) rinfo;
    (void) ec;
}

std::size_t
nvml_dax::get_size(const resource_info_ptr& rinfo, std::error_code& ec) const {
    (void) rinfo;
    (void) ec;

    return 0; //XXX
}

bool nvml_dax::accepts(resource_info_ptr res) const {
    switch(res->type()) {
        case data::resource_type::local_posix_path:
            return true;
        default:
            return false;
    }
}

std::string nvml_dax::to_string() const {
    return "NORNS_NVML(\"" + m_mount.string() + "\", " + std::to_string(m_quota) + ")";
}

} // namespace storage
} // namespace norns
