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

#include <sys/socket.h>
#include "process-credentials.hpp"

namespace norns {
namespace auth {

#if 0
boost::optional<credentials> 
credentials::fetch(const ba::ip::tcp::socket& socket) {

    using SocketType = ba::ip::tcp::socket;

    struct ucred ucred;
    socklen_t len = sizeof(ucred);

    int sockfd = const_cast<SocketType&>(socket).native_handle();

    if(getsockopt(sockfd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) != -1) {
        return credentials(ucred.pid, ucred.uid, ucred.gid);
    }

    return boost::none;

}

boost::optional<credentials> 
credentials::fetch(const ba::local::stream_protocol::socket& socket) {

    using SocketType = ba::local::stream_protocol::socket;

    struct ucred ucred;
    socklen_t len = sizeof(ucred);

    int sockfd = const_cast<SocketType&>(socket).native_handle();

    if(getsockopt(sockfd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) != -1) {
        return credentials(ucred.pid, ucred.uid, ucred.gid);
    }

    return boost::none;

}
#endif

credentials::credentials(pid_t pid, uid_t uid, gid_t gid) :
    m_pid(pid),
    m_uid(uid),
    m_gid(gid) {}

pid_t credentials::pid() const {
    return m_pid;
}

uid_t credentials::uid() const {
    return m_uid;
}

gid_t credentials::gid() const {
    return m_gid;
}

} // namespace auth
} // namespace norns
