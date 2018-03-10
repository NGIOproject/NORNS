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

#include "resource.hpp"

#ifndef __LOCAL_PATH_HPP__
#define __LOCAL_PATH_HPP__

namespace data {

/*! Local filesystem path data */
struct local_path : public resource_info {

    local_path(std::string nsid, std::string datapath);
    ~local_path();
    resource_type type() const override;
    std::string nsid() const override;
    bool is_remote() const override;
    std::string to_string() const override;

    std::string datapath() const;

    std::string m_nsid;
    std::string m_datapath;
};


namespace detail {

template <>
struct resource_impl<resource_type::local_posix_path> : public resource {

    resource_impl(std::shared_ptr<resource_info> base_info);
    std::string to_string() const override;
//    resource_type type() const override;
    std::shared_ptr<resource_info> info() const override;
    std::shared_ptr<storage::backend> backend() const override;
    void set_backend(const std::shared_ptr<storage::backend> backend) override;

    std::shared_ptr<storage::backend> m_backend;
    std::shared_ptr<local_path> m_resource_info;
};

template <>
struct stream_impl<resource_type::local_posix_path> : public data::stream {
    stream_impl(std::shared_ptr<resource> resource, stream_type type);
    ~stream_impl();
    std::size_t read(buffer& b) override;
    std::size_t write(const buffer& b) override;

    int m_fd = -1;

};

} // namespace detail

using local_path_resource = detail::resource_impl<resource_type::local_posix_path>;
using local_path_stream = detail::stream_impl<resource_type::local_posix_path>;

} // namespace data

#endif /* __LOCAL_PATH_HPP__ */
