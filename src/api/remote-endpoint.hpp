#ifndef __REMOTE_ENDPOINT_HPP__
#define __REMOTE_ENDPOINT_HPP__

#include <memory>
#include <boost/asio.hpp>

#include "api/session.hpp"

namespace ba = boost::asio;
namespace bfs = boost::filesystem;

namespace norns {
namespace api {

template <typename Message>
class remote_endpoint {

    using SessionType = session<ba::ip::tcp::socket, Message>;
    using DispatcherType = typename SessionType::Dispatcher;

public: 
    remote_endpoint(short port, ba::io_service& ios,
                    std::shared_ptr<DispatcherType> dispatcher) :
        m_port(port),
        m_socket(ios),
        m_acceptor(ios, ba::ip::tcp::endpoint(ba::ip::tcp::v4(), port)),
        m_dispatcher(dispatcher) {

        m_acceptor.set_option(ba::ip::tcp::acceptor::reuse_address(true));
    }

    ~remote_endpoint() { }

    void do_accept() {
        m_acceptor.async_accept(m_socket, 
            [this](const boost::system::error_code& ec) {
                if(!ec) {
                    std::make_shared<SessionType>(
                            std::move(m_socket),
                            m_dispatcher
                        )->start();
                }

                do_accept();
            }
        );
    }

private:
    short m_port;
    ba::ip::tcp::socket m_socket;
    ba::ip::tcp::acceptor m_acceptor;
    std::shared_ptr<DispatcherType> m_dispatcher;
};

} // namespace api
} // namespace norns

#endif /* __REMOTE_ENDPOINT_HPP__ */
