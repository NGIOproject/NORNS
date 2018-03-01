/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 *                                                                       *
 * This file is part of the Data Scheduler, a daemon for tracking and    *
 * managing requests for asynchronous data transfer in a hierarchical    *
 * storage environment.                                                  *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The Data Scheduler is free software: you can redistribute it and/or   *
 * modify it under the terms of the GNU Lesser General Public License    *
 * as published by the Free Software Foundation, either version 3 of     *
 * the License, or (at your option) any later version.                   *
 *                                                                       *
 * The Data Scheduler is distributed in the hope that it will be useful, *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with Data Scheduler.  If not, see                *
 * <http://www.gnu.org/licenses/>.                                       *
 *                                                                       *
 *************************************************************************/

#include "fake-daemon.hpp"

#include "catch.hpp"

#include <set>
#include <unordered_map>
#include <chrono>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <norns.h>
#include <urd.hpp>
#include <settings.hpp>

#include "fake-daemon.hpp"

namespace bfs = boost::filesystem;

//#define USE_REAL_DAEMON

namespace {

struct test_env {

    ~test_env() {

        if(m_dirs.size() != 0) {
            for(const auto& kv : m_dirs) {
                const auto nsid = kv.first;
                const auto abs_mnt = kv.second;
                bfs::remove_all(abs_mnt);
            }
            m_dirs.clear();
        }

        if(m_backends.size() != 0) {
            for(const auto& nsid : m_backends) {
                teardown_backend(nsid);
            }
            m_backends.clear();
        }
    }

    // NOTE: rel_mount must be relative to the CWD
    void create_backend(std::string nsid, bfs::path rel_mnt, size_t quota) {

        auto abs_mnt = bfs::absolute(rel_mnt);

        bool res = bfs::create_directory(abs_mnt);
        REQUIRE(res == true);

        m_dirs.emplace(nsid, abs_mnt);

        struct norns_cred cred;
        norns_backend_t bend = NORNS_BACKEND(nsid.c_str(), NORNS_BACKEND_POSIX_FILESYSTEM, abs_mnt.c_str(), quota);
        norns_error_t rv = norns_register_backend(&cred, &bend);
        REQUIRE(rv == NORNS_SUCCESS);

        m_backends.emplace(nsid);

    }

    void create_input_files(bfs::path name, bfs::path rel_mnt, std::size_t size) {

        auto parent_dirs = name.parent_path();
        auto abs_mnt = bfs::absolute(rel_mnt);

        if(!parent_dirs.empty() && parent_dirs != ".") {
            bool res = bfs::create_directories(abs_mnt / parent_dirs);
            REQUIRE(res == true);
        }

        auto fname = abs_mnt / name;

        bfs::ofstream file(fname, std::ios_base::out | std::ios_base::binary);

        REQUIRE(file);

        if(size != 0) {
            std::vector<uint64_t> data(size/sizeof(uint64_t), 42);
            file.write(reinterpret_cast<const char*>(&data[0]), size);
        }
    }

    void teardown_backend(const std::string nsid) {
        if(m_backends.count(nsid) != 0) {
            struct norns_cred cred;
            norns_error_t rv = norns_unregister_backend(&cred, nsid.c_str());
            REQUIRE(rv == NORNS_SUCCESS);
        }
    }

    std::set<std::string> m_backends;
    std::unordered_map<std::string, bfs::path> m_dirs;
};

}

SCENARIO("copy local data", "[api::norns_submit_copy_local]") {
    GIVEN("a running urd instance") {

#ifndef USE_REAL_DAEMON
        fake_daemon td;
        td.run();
#endif

        test_env env;

        const char* src_nsid = "tmp0";
        const char* src_mnt = "./tmp0";
        const char* dst_nsid = "tmp1";
        const char* dst_mnt = "./tmp1";

        env.create_backend(src_nsid, src_mnt, 16384);
        env.create_backend(dst_nsid, dst_mnt, 16384);

        /**************************************************************************************************************/
        /* tests for error conditions                                                                                 */
        /**************************************************************************************************************/
        // TODO


        /**************************************************************************************************************/
        /* tests for NORNS_IOTASK_COPY                                                                                */
        /**************************************************************************************************************/
        WHEN("copying a single file from NORNS_LOCAL_PATH to NORNS_LOCAL_PATH") {
            
            norns_op_t task_op = NORNS_IOTASK_COPY;

            const char* src_path = "/b/c/d/f0";
            env.create_input_files(src_path, src_mnt, 4096);
            //const char* dst_path = "/a/b/c/f0";
            const char* dst_path = "f1";

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            norns_error_t rv = norns_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // wait until the task completes
            // TODO: replace with norns_wait() once it's available
            sleep(15);
            
        }

#ifndef USE_REAL_DAEMON
        int ret = td.stop();
        REQUIRE(ret == 0);
#endif
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("attempting to request a transfer") {

            norns_iotask_t task = NORNS_IOTASK(
                NORNS_IOTASK_COPY, 
                NORNS_LOCAL_PATH("nvml0://", "/a/b/c/"),
                NORNS_REMOTE_PATH("nvml0://", "node1", "/a/b/d/"));

            norns_error_t rv = norns_submit(&task);

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
#endif
}
