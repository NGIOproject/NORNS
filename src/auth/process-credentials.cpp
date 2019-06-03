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

credentials::credentials() :
    m_pid(0),
    m_uid(0),
    m_gid(0) {}

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
