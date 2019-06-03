#ifndef __API_LISTENER_HPP__
#define __API_LISTENER_HPP__

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include "common.hpp"
#include "api/local-endpoint.hpp"
#include "api/remote-endpoint.hpp"
#include "api/signal-listener.hpp"

namespace ba = boost::asio;
namespace bfs = boost::filesystem;

namespace norns {
namespace api {

/* simple lister for an AF_UNIX socket that accepts requests asynchronously and
 * invokes a callback with a fixed-length payload */
template <typename Message>
class listener {

    using Dispatcher = dispatch_table<
        typename Message::key_type, 
        typename Message::key_hash_type, 
        std::unique_ptr<typename Message::response_type>,
        std::unique_ptr<typename Message::request_type>
            >;

    using MessageKey = typename Message::key_type;

public:
    explicit listener() :
        m_ios(),
        m_msg_handlers(std::make_shared<Dispatcher>()), 
        m_signal_listener(m_ios) { }

    ~listener() {
        Message::cleanup();
    }

    void run() {

        for(auto& endp : m_local_endpoints) {
            endp->do_accept();
        }

        for(auto& endp : m_remote_endpoints) {
            endp->do_accept();
        }

        m_signal_listener.do_accept();

        m_ios.run();
    }

    void stop() {
        m_ios.stop();
    }

    template <typename Callable>
    void register_callback(MessageKey k, Callable&& func) {
        m_msg_handlers->add(k, std::forward<Callable>(func));
    }

    /* register a socket endpoint from which to accept local requests */
    void register_endpoint(const bfs::path& sockfile) {
        m_local_endpoints.emplace_back(
                std::make_shared<local_endpoint<Message>>(sockfile, m_ios, m_msg_handlers));
    }

    /* register a socket endpoint from which to accept remote requests */
    void register_endpoint(short port) {
        m_remote_endpoints.emplace_back(
                std::make_shared<remote_endpoint<Message>>(port, m_ios, m_msg_handlers));
    }

    template <typename... Args>
    void set_signal_handler(signal_listener::SignalHandler handler, Args... signums) {
        m_signal_listener.set_handler(handler, std::forward<Args>(signums)...);
    }

    static void cleanup() {
        Message::cleanup();
    }

private:
    /*! Main io_service */
    boost::asio::io_service                m_ios;

    /*! Dispatcher of message handlers */
    std::shared_ptr<Dispatcher> m_msg_handlers;

    /*! Dispatcher of signal listener */
    signal_listener m_signal_listener;

    /*! Local endpoints */
    std::vector<std::shared_ptr<local_endpoint<Message>>>  m_local_endpoints;

    /*! Remote endpoints */
    std::vector<std::shared_ptr<remote_endpoint<Message>>> m_remote_endpoints;
};

} // namespace api
} // namespace norns

#endif /* __API_LISTENER_HPP__ */
