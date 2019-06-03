#ifndef NORNS_CONTEXT_HPP
#define NORNS_CONTEXT_HPP

#include <boost/filesystem.hpp>
#include <memory>

namespace bfs = boost::filesystem;

namespace hermes {
    class async_engine;
} // namespace hermes

namespace norns {

struct context {

    context(bfs::path staging_directory,
            std::shared_ptr<hermes::async_engine> network_service) :
        m_staging_directory(std::move(staging_directory)),
        m_network_service(std::move(network_service)) { }

    bfs::path 
    staging_directory() const {
        return m_staging_directory;
    }

    std::shared_ptr<hermes::async_engine>
    network_service() const {
        return m_network_service;
    }

    bfs::path m_staging_directory;
    std::shared_ptr<hermes::async_engine> m_network_service;
};

} // namespace norns

#endif // NORNS_CONTEXT_HPP
