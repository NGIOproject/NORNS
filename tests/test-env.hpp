#ifndef __TEST_ENV_HPP__
#define __TEST_ENV_HPP__

#include <string>
#include <set>
#include <unordered_map>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include "fake-daemon.hpp"

namespace bfs = boost::filesystem;

// struct fake_daemon;
// struct fake_daemon_cfg;

struct test_env {

    test_env(bool requires_remote_peer = false);
    test_env(const fake_daemon_cfg& cfg);
    ~test_env();

    bfs::path basedir() const;

    std::tuple<const char*, bfs::path> create_namespace(const std::string& nsid, const bfs::path& mnt, size_t quota);
    bfs::path add_to_namespace(const std::string& nsid, const bfs::path& dirname);
    bfs::path add_to_namespace(const std::string& nsid, const bfs::path& filename, std::size_t size);
    bfs::path add_to_namespace(const std::string& nsid, const bfs::path& src, const bfs::path& linkname);
    bfs::path get_from_namespace(const std::string& nsid, const bfs::path& name) const;
    bfs::path create_directory(const bfs::path& dirname, const bfs::path& parent);
    bfs::path create_file(const bfs::path& name, const bfs::path& parent, std::size_t size);
    bfs::path create_symlink(const bfs::path& src_name, const bfs::path& link_name, const bfs::path& parent);

    void remove_access(const bfs::path& p);

    void teardown_namespace(const std::string nsid);

    void notify_success();
    void cleanup();

    bool m_test_succeeded;
    std::string m_uid;
    bfs::path m_base_dir;

#ifndef USE_REAL_DAEMON
    fake_daemon m_ltd;
    fake_daemon m_rtd;
#endif

    std::set<bfs::path> m_dirs;
    std::unordered_map<std::string, bfs::path> m_namespaces;
    std::vector<bfs::path> m_unacessible_paths;
};

#endif // __TEST_ENV_HPP__
