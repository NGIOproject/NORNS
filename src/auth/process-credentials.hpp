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
