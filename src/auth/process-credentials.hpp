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

#ifndef __PROCESS_CREDENTIALS_HPP__
#define __PROCESS_CREDENTIALS_HPP__

#include <sys/types.h>
#include <boost/asio.hpp>
#include <boost/optional.hpp>

namespace ba = boost::asio;

namespace norns {
namespace auth {

struct credentials {

    template <typename SocketType>
    static boost::optional<credentials> fetch(const SocketType& socket) {

        struct ucred ucred;
        socklen_t len = sizeof(ucred);

        int sockfd = const_cast<SocketType&>(socket).native_handle();

        if(getsockopt(sockfd, SOL_SOCKET, SO_PEERCRED, &ucred, &len) != -1) {
            return credentials(ucred.pid, ucred.uid, ucred.gid);
        }

        return boost::none;
    }

    credentials();
    credentials(pid_t pid, uid_t uid, gid_t gid);

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
