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

    test_env();
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
    fake_daemon m_td;
#endif

    std::set<bfs::path> m_dirs;
    std::unordered_map<std::string, bfs::path> m_namespaces;
    std::vector<bfs::path> m_unacessible_paths;
};

#endif // __TEST_ENV_HPP__
