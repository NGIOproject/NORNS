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

#ifndef __PROCESS_CREDENTIALS_HPP__
#define __PROCESS_CREDENTIALS_HPP__

#include <sys/types.h>
#include <boost/asio.hpp>
#include <boost/optional.hpp>

namespace ba = boost::asio;

namespace norns {
namespace auth {

struct credentials {

    static boost::optional<credentials> fetch(const ba::generic::stream_protocol::socket& socket);

private:
    credentials(pid_t pid, uid_t uid, gid_t gid);

public:
    pid_t pid() const;
    uid_t uid() const;
    gid_t gid() const;

    pid_t m_pid;
    uid_t m_uid;
    gid_t m_gid;
};

} // namespace auth
} // namespace norns

#endif /* __PROCESS_CREDENTIALS_HPP__ */
