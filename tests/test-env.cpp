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

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/regex.hpp>

#include "catch.hpp"
#include "test-env.hpp"
#include "norns.h"
#include "nornsctl.h"
#include "config-template.hpp"

//#define __DEBUG_OUTPUT__

namespace bfs = boost::filesystem;

namespace {

std::string generate_testid() {
    const auto seed = boost::uuids::to_string(boost::uuids::random_generator()());
    return std::string("test_") + seed.substr(0, 8);
}

#ifndef USE_REAL_DAEMON
bfs::path create_config_file(const bfs::path& basedir) {

    const bfs::path cfgdir = basedir / "config";
    bool res = bfs::create_directory(cfgdir);
    REQUIRE(res == true);

    const bfs::path config_file = cfgdir / "test.conf";
    bfs::ofstream outf(config_file);
    outf << boost::regex_replace(config_file::cftemplate,
                              boost::regex("@localstatedir@"),
                              cfgdir.string());
    outf.close();

    return config_file;
}

bfs::path patch_libraries(const bfs::path& basedir) {

    const auto config_file = create_config_file(basedir);

    int rv = ::setenv("NORNS_CONFIG_FILE", config_file.c_str(), 1);
    REQUIRE(rv != -1);

    libnorns_reload_config_file();
    libnornsctl_reload_config_file();

    return config_file;
}
#endif

}

test_env::test_env() :
    m_test_succeeded(false),
    m_uid(generate_testid()),
    m_base_dir(bfs::absolute(bfs::current_path() / m_uid)) {

    bool res = bfs::create_directory(m_base_dir);
    REQUIRE(res == true);

#ifndef USE_REAL_DAEMON
    const auto config_file = patch_libraries(m_base_dir);
    m_td.configure(config_file);
    m_td.run();
#endif

}

test_env::test_env(const fake_daemon_cfg& cfg) : 
    m_test_succeeded(false),
    m_uid(generate_testid()),
    m_base_dir(bfs::absolute(bfs::current_path() / m_uid)) {

    bool res = bfs::create_directory(m_base_dir);
    REQUIRE(res == true);

#ifndef USE_REAL_DAEMON
    const auto config_file = patch_libraries(m_base_dir);
    m_td.configure(config_file, cfg);
    m_td.run();
#else
    (void) cfg;
#endif

}

bfs::path test_env::basedir() const {
    return m_base_dir;
}

void test_env::cleanup() {

    if(m_unacessible_paths.size() != 0) {
        for(const auto& p : m_unacessible_paths) {
            bfs::permissions(p, bfs::add_perms | bfs::all_all);
        }
    }

    if(m_dirs.size() != 0) {
        for(const auto& dir : m_dirs) {
            bfs::remove_all(dir);
        }
        m_dirs.clear();
    }

    bfs::remove_all(m_base_dir);
}

test_env::~test_env() {

    if(m_namespaces.size() != 0) {
        for(const auto& kv : m_namespaces) {
            const auto nsid = kv.first;
            const auto dir = kv.second;
            teardown_namespace(nsid);
        }
        m_namespaces.clear();
    }

#ifndef USE_REAL_DAEMON
    int ret = m_td.stop();
    //REQUIRE(ret == 0);
#endif

    if(m_test_succeeded) {
        cleanup();
    }
}

void test_env::notify_success() {
    m_test_succeeded = true;
}

std::tuple<const char*, bfs::path> test_env::create_namespace(std::string nsid, bfs::path mnt, size_t quota) {

    auto abs_dir = create_directory(mnt, m_base_dir);

    nornsctl_backend_t ns = NORNSCTL_BACKEND(NORNS_BACKEND_POSIX_FILESYSTEM, 
                                       abs_dir.c_str(), quota);
    norns_error_t rv = nornsctl_register_namespace(nsid.c_str(), &ns);
    REQUIRE(rv == NORNS_SUCCESS);

    m_namespaces.emplace(nsid, abs_dir);

    return std::make_tuple(nsid.c_str(), abs_dir);
}

bfs::path test_env::add_to_namespace(const std::string& nsid, const bfs::path& dirname) {

    const auto& it = m_namespaces.find(nsid);

    if(it == m_namespaces.end()) {
        throw std::invalid_argument("namespace " + nsid + " is not registered!");
    }

    const auto& abs_base_dir = it->second;
    return create_directory(dirname, abs_base_dir);
}

bfs::path test_env::add_to_namespace(const std::string& nsid, const bfs::path& filename, std::size_t size) {

    const auto& it = m_namespaces.find(nsid);

    if(it == m_namespaces.end()) {
        throw std::invalid_argument("namespace " + nsid + " is not registered!");
    }

    const auto& abs_base_dir = it->second;
    return create_file(filename, abs_base_dir, size);
}

bfs::path test_env::add_to_namespace(const std::string& nsid, const bfs::path& target, const bfs::path& linkname) {

    const auto& it = m_namespaces.find(nsid);

    if(it == m_namespaces.end()) {
        throw std::invalid_argument("namespace " + nsid + " is not registered!");
    }

    const auto& abs_base_dir = it->second;
    return create_symlink(target, linkname, abs_base_dir);
}


bfs::path test_env::get_from_namespace(const std::string& nsid, const bfs::path& name) const {
    const auto& it = m_namespaces.find(nsid);

    if(it == m_namespaces.end()) {
        throw std::invalid_argument("namespace " + nsid + " is not registered!");
    }

    const auto& abs_base_dir = it->second;

    return bfs::canonical(abs_base_dir / name);
}


bfs::path test_env::create_file(const bfs::path& name, const bfs::path& parent, std::size_t size) {

    auto parent_dirs = bfs::relative("/", name.parent_path());
    auto abs_mnt = bfs::absolute(parent);

    if(!parent_dirs.empty() && parent_dirs != "." && parent_dirs != "/") {
        boost::system::error_code ec;
        bfs::create_directories(abs_mnt / parent_dirs, ec);

        REQUIRE(!ec);
    }

    auto fname = abs_mnt / name;

#ifdef __DEBUG_OUTPUT__
    std::cerr << "touch " << fname << "\n";
#endif // __DEBUG_OUTPUT__

    bfs::ofstream file(fname, std::ios_base::out | std::ios_base::binary);

    REQUIRE(file);

    if(size != 0) {
        std::vector<uint64_t> data(size/sizeof(uint64_t), 42);
        file.write(reinterpret_cast<const char*>(&data[0]), size);
    }

    return fname;
}

bfs::path test_env::create_symlink(const bfs::path& target_name, 
                                   const bfs::path& link_name, 
                                   const bfs::path& parent) {

    boost::system::error_code ec;
    auto abs_mnt = bfs::absolute(parent);
    auto abs_target_path = bfs::canonical(abs_mnt / target_name, ec);
    REQUIRE(!ec);

    auto parent_dirs = bfs::relative("/", link_name.parent_path());

    if(!parent_dirs.empty() && parent_dirs != "." && parent_dirs != "/") {
        bfs::create_directories(abs_mnt / parent_dirs, ec);

        REQUIRE(!ec);
    }

    auto abs_link_name = abs_mnt / link_name;

#ifdef __DEBUG_OUTPUT__
    std::cerr << "ln -s " << abs_target_path << " " << abs_link_name << "\n";
#endif // __DEBUG_OUTPUT__

    bfs::create_symlink(abs_target_path, abs_link_name, ec);
    REQUIRE(!ec);

    return abs_link_name;
}

bfs::path test_env::create_directory(const bfs::path& dirname, const bfs::path& parent) {

    auto abs_dirname = bfs::absolute(parent / dirname);

#ifdef __DEBUG_OUTPUT__
    std::cerr << "mkdir -p " << abs_dirname << "\n";
#endif // __DEBUG_OUTPUT__

    boost::system::error_code ec;
    bool res = bfs::create_directories(abs_dirname);
    REQUIRE(!ec);

    m_dirs.emplace(abs_dirname);

    return abs_dirname;
}

void test_env::remove_access(const bfs::path& p) {
    bfs::permissions(p, bfs::remove_perms | bfs::all_all);
    m_unacessible_paths.push_back(p);
}

void test_env::teardown_namespace(const std::string nsid) {
    if(m_namespaces.count(nsid) != 0) {
        norns_error_t rv = nornsctl_unregister_namespace(nsid.c_str());

        // cannot use REQUIRE here since it may throw an exception,
        // and throwing exceptions from destructors is a huge problem
        //REQUIRE(rv == NORNS_SUCCESS);
    }
}
