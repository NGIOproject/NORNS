#ifndef __LOCAL_ENDPOINT_HPP__
#define __LOCAL_ENDPOINT_HPP__

#include <memory>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "api/session.hpp"
#include "logger.hpp"

namespace ba = boost::asio;
namespace bfs = boost::filesystem;

namespace norns {
namespace api {

template <typename Message>
class local_endpoint {

    using SessionType = session<ba::local::stream_protocol::socket, Message>;
    using DispatcherType = typename SessionType::Dispatcher;

public:
    local_endpoint(const bfs::path& sockfile, ba::io_service& ios, 
                   std::shared_ptr<DispatcherType> dispatcher) :
        m_sockfile(sockfile),
        m_socket(ios),
        m_acceptor(ios, ba::local::stream_protocol::endpoint(sockfile.string())),
        m_dispatcher(dispatcher) { }

    ~local_endpoint() {
        boost::system::error_code ec;
        bfs::remove(m_sockfile, ec);

        if(ec) {
            LOGGER_ERROR("Failed to remove sockfile {}: {}", 
                    m_sockfile, ec.message());
        }
    }

    void do_accept() {
        m_acceptor.async_accept(m_socket, 
            [this](const boost::system::error_code& ec) {
                if(!ec) {
                    std::make_shared<SessionType>(
                            std::move(m_socket), m_dispatcher
                        )->start();
                }

                do_accept();
            }
        );
    }

private:
    bfs::path m_sockfile;
    ba::local::stream_protocol::socket m_socket;
    ba::local::stream_protocol::acceptor m_acceptor;
    std::shared_ptr<DispatcherType> m_dispatcher;
};

} // namespace api
} // namespace norns

#endif /* __LOCAL_ENDPOINT_HPP__ */
