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

#include <mpi.h>
#include "norns.h"
#include "nornsctl.h"
#include "test-env.hpp"
#include "catch.hpp"

/******************************************************************************/
/* tests for push transfers (single files)                                    */
/******************************************************************************/
SCENARIO("copy local POSIX file to remote POSIX file", 
         "[mpi::norns_submit_push_to_posix_file]") {

    GIVEN("two running urd instances (local and remote)") {

        /**********************************************************************/
        /* setup common environment                                           */
        /**********************************************************************/
        struct {
            // define input names
            const bfs::path src_file_at_root = "/file0";
            const bfs::path src_file_at_subdir = "/a/b/c/d/file0";
            const bfs::path src_invalid_file = "/a/b/c/d/does_not_exist_file0";
            const bfs::path src_invalid_dir = "/a/b/c/d/does_not_exist_dir0";
            const bfs::path src_subdir0 = "/input_dir0";
            const bfs::path src_subdir1 = "/input_dir0/a/b/c/input_dir1";
            const bfs::path src_empty_dir = "/empty_dir0";

            const bfs::path src_noperms_file0 = "/noperms_file0";
            const bfs::path src_noperms_file1 = "/noperms/a/b/c/d/noperms_file0"; // parents accessible
            const bfs::path src_noperms_file2 = "/noperms/noperms_subdir0/file0"; // parents non-accessible
            const bfs::path src_noperms_subdir0 = "/noperms_subdir0"; // subdir non-accessible
            const bfs::path src_noperms_subdir1 = "/noperms/a/b/c/d/noperms_subdir1"; // child subdir non-accessible
            const bfs::path src_noperms_subdir2 = "/noperms/noperms_subdir2/a"; // parent subdir non-accessible

            const bfs::path src_symlink_at_root0 = "/symlink0";
            const bfs::path src_symlink_at_root1 = "/symlink1";
            const bfs::path src_symlink_at_root2 = "/symlink2";
            const bfs::path src_symlink_at_subdir0 = "/foo/bar/baz/symlink0";
            const bfs::path src_symlink_at_subdir1 = "/foo/bar/baz/symlink1";
            const bfs::path src_symlink_at_subdir2 = "/foo/bar/baz/symlink2";

            const bfs::path dst_root            = "/";
            const bfs::path dst_subdir0         = "/output_dir0";
            const bfs::path dst_subdir1         = "/output_dir1";
            const bfs::path dst_file_at_root0   = "/file0"; // same basename
            const bfs::path dst_file_at_root1   = "/file1"; // different basename
            const bfs::path dst_file_at_subdir0 = "/a/b/c/d/file0"; // same fullname
            const bfs::path dst_file_at_subdir1 = "/a/b/c/d/file1"; // same parents, different basename
            const bfs::path dst_file_at_subdir2 = "/e/f/g/h/i/file0"; // different parents, same basename
            const bfs::path dst_file_at_subdir3 = "/e/f/g/h/i/file1"; // different fullname
        } test_context;

        test_env env(false);

        const char* remote_host = "127.0.0.1:42000";

        // Get the number of processes
        int world_size;
        if(MPI_Comm_size(MPI_COMM_WORLD, &world_size) != MPI_SUCCESS) {
            FAIL("Failed to determine number of processes");
        }

        // Get the rank of the process
        int world_rank;
        if(MPI_Comm_rank(MPI_COMM_WORLD, &world_rank) != MPI_SUCCESS) {
            FAIL("Failed to determine own rank");
        }

        // code for the test client
        auto client = [world_rank, &env, &test_context] {
            std::string nsid("client");
            bfs::path mountdir;

            // create namespaces
            std::tie(std::ignore, mountdir) = 
                env.create_namespace(nsid, "mnt/" + nsid, 16384);

            // create input data
            env.add_to_namespace(nsid, test_context.src_file_at_root, 40000);
            env.add_to_namespace(nsid, test_context.src_file_at_subdir, 80000);
            env.add_to_namespace(nsid, test_context.src_subdir0);
            env.add_to_namespace(nsid, test_context.src_subdir1);
            env.add_to_namespace(nsid, test_context.src_empty_dir);

            for(int i=0; i<10; ++i) {
                const bfs::path p{test_context.src_subdir0 /
                                  ("file" + std::to_string(i))};
                env.add_to_namespace(nsid, p, 4096+i*10);
            }

            for(int i=0; i<10; ++i) {
                const bfs::path p{test_context.src_subdir1 /
                                  ("file" + std::to_string(i))};
                env.add_to_namespace(nsid, p, 4096+i*10);
            }

            // create input data with special permissions
            auto p = 
                env.add_to_namespace(nsid, test_context.src_noperms_file0, 0);
            env.remove_access(p);

            p = env.add_to_namespace(nsid, test_context.src_noperms_file1, 0);
            env.remove_access(p);

            p = env.add_to_namespace(nsid, test_context.src_noperms_file2, 0);
            env.remove_access(p.parent_path());

            p = env.add_to_namespace(nsid, test_context.src_noperms_subdir0);
            env.remove_access(p);

            p = env.add_to_namespace(nsid, test_context.src_noperms_subdir1);
            env.remove_access(p);

            p = env.add_to_namespace(nsid, test_context.src_noperms_subdir2);
            env.remove_access(p.parent_path());

            // add symlinks to the namespace
            env.add_to_namespace(nsid, test_context.src_file_at_root,
                                 test_context.src_symlink_at_root0);
            env.add_to_namespace(nsid, test_context.src_subdir0,
                                 test_context.src_symlink_at_root1);
            env.add_to_namespace(nsid, test_context.src_subdir1,
                                 test_context.src_symlink_at_root2);

            env.add_to_namespace(nsid, test_context.src_file_at_root,
                                 test_context.src_symlink_at_subdir0);
            env.add_to_namespace(nsid, test_context.src_subdir0,
                                 test_context.src_symlink_at_subdir1);
            env.add_to_namespace(nsid, test_context.src_subdir1,
                                 test_context.src_symlink_at_subdir2);

            // manually create a symlink leading outside namespace 0
            boost::system::error_code ec;
            const bfs::path out_symlink = "/out_symlink";
            bfs::create_symlink(env.basedir(), mountdir / out_symlink, ec);
            REQUIRE(!ec);
        };

        // code for the test servers
        auto server = [world_rank, &env, &test_context] {
            std::string nsid("server" + std::to_string(world_rank));
            bfs::path mountdir;

            // create namespaces
            std::tie(std::ignore, mountdir) = 
                env.create_namespace(nsid, "mnt/" + nsid, 16384);

            // create required output directories
            env.add_to_namespace(nsid, test_context.dst_subdir1);
        };


        // register code
        std::vector<std::function<void()>> functors;
        functors.reserve(world_size);

        functors.push_back(client);

        for(int i = 1; i < world_size; ++i) {
            functors.push_back(server);
        }

        // run registered code for each process
        functors.at(world_rank)();

        // wait for everyone to finish preparing the test environment
        MPI_Barrier(MPI_COMM_WORLD);

        if(world_rank != 0) {
            // servers do nothing but wait for client to complete
            MPI_Barrier(MPI_COMM_WORLD);
            env.notify_success();
            return;
        }

        /**********************************************************************/
        /* begin tests                                                        */
        /**********************************************************************/
        // cp -r /a/contents.* -> / = /contents.*
        WHEN("copying the contents of a NORNS_LOCAL_PATH subdir from SRC "
             "namespace's root to DST namespace's root\n"
             "  cp -r /a/contents.* -> / = /contents.* ") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH("client", 
                                              test_context.src_subdir0.c_str()),
                             NORNS_REMOTE_PATH("server1", 
                                               remote_host, 
                                               test_context.dst_root.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("norns_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = norns_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

#if 0
                        THEN("Copied files are identical to original") {
                            bfs::path src = 
                                env.get_from_namespace("client", src_subdir0);
                            bfs::path dst = 
                                env.get_from_namespace("server1", dst_root);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
#endif
                    }
                }
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        env.notify_success();
    }
}
