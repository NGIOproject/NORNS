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

#include <boost/filesystem/fstream.hpp>
#include "mpi-helpers.hpp"
#include "commands.hpp"
#include "norns.h"
#include "nornsctl.h"
#include "test-env.hpp"
#include "catch.hpp"


extern std::vector< std::pair<std::string, int> > mpi::test_hosts;

namespace test_data {

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
    const bfs::path dst_file_at_root2   = "/file2";
    const bfs::path dst_file_at_root3   = "/file3";
    const bfs::path dst_file_at_subdir0 = "/a/b/c/d/file0"; // same fullname
    const bfs::path dst_file_at_subdir1 = "/a/b/c/d/file1"; // same parents, different basename
    const bfs::path dst_file_at_subdir2 = "/e/f/g/h/i/file0"; // different parents, same basename
    const bfs::path dst_file_at_subdir3 = "/e/f/g/h/i/file1"; // different fullname
    const bfs::path dst_file_at_subdir4 = "/a/b/c/d/file4";
    const bfs::path dst_file_at_subdir5 = "/a/b/c/d/file5";
} context;

}

/******************************************************************************/
/* tests for push transfers (single files)                                    */
/******************************************************************************/
SCENARIO("copy local POSIX file to remote POSIX file", 
         "[mpi::norns_submit_push_to_posix_file]") {

    auto ctx = test_data::context;

    GIVEN("a local running urd instances and several remote ones)") {

        /**********************************************************************/
        /* setup common environment                                           */
        /**********************************************************************/
        const std::string bind_address(
            mpi::test_hosts.at(mpi::get_rank()).first + ":" +
            std::to_string(mpi::test_hosts.at(mpi::get_rank()).second));

        const std::string remote_host(
            mpi::test_hosts.at(1).first + ":" +
            std::to_string(mpi::test_hosts.at(1).second));

        if(::setenv("TEST_FORCE_BIND_ADDRESS", bind_address.c_str(), 1)) {
            throw std::runtime_error(
                "Failed to set TEST_FORCE_BIND_ADDRESS env var");
        }

        test_env env;


        std::string nsid("client");
        bfs::path mountdir;

        // create namespaces
        std::tie(std::ignore, mountdir) = 
            env.create_namespace(nsid, "mnt/" + nsid, 16384);

        // create input data
        env.add_to_namespace(nsid, ctx.src_file_at_root, 40000);
        env.add_to_namespace(nsid, ctx.src_file_at_subdir, 80000);
        env.add_to_namespace(nsid, ctx.src_subdir0);
        env.add_to_namespace(nsid, ctx.src_subdir1);
        env.add_to_namespace(nsid, ctx.src_empty_dir);

        for(int i=0; i<10; ++i) {
            const bfs::path p{ctx.src_subdir0 /
                                ("subfile" + std::to_string(i))};
            env.add_to_namespace(nsid, p, 4096+i*10);
        }

        for(int i=0; i<10; ++i) {
            const bfs::path p{ctx.src_subdir1 /
                                ("subfile" + std::to_string(i))};
            env.add_to_namespace(nsid, p, 4096+i*10);
        }

        // create input data with special permissions
        auto p = 
            env.add_to_namespace(nsid, ctx.src_noperms_file0, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid, ctx.src_noperms_file1, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid, ctx.src_noperms_file2, 0);
        env.remove_access(p.parent_path());

        p = env.add_to_namespace(nsid, ctx.src_noperms_subdir0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid, ctx.src_noperms_subdir1);
        env.remove_access(p);

        p = env.add_to_namespace(nsid, ctx.src_noperms_subdir2);
        env.remove_access(p.parent_path());

        // add symlinks to the namespace
        env.add_to_namespace(nsid, ctx.src_file_at_root,
                                ctx.src_symlink_at_root0);
        env.add_to_namespace(nsid, ctx.src_subdir0,
                                ctx.src_symlink_at_root1);
        env.add_to_namespace(nsid, ctx.src_subdir1,
                                ctx.src_symlink_at_root2);

        env.add_to_namespace(nsid, ctx.src_file_at_root,
                                ctx.src_symlink_at_subdir0);
        env.add_to_namespace(nsid, ctx.src_subdir0,
                                ctx.src_symlink_at_subdir1);
        env.add_to_namespace(nsid, ctx.src_subdir1,
                                ctx.src_symlink_at_subdir2);

        // manually create a symlink leading outside namespace 0
        boost::system::error_code ec;
        const bfs::path out_symlink = "/out_symlink";
        bfs::create_symlink(env.basedir(), mountdir / out_symlink, ec);
        REQUIRE(!ec);

        const std::string nsid0("client");
        const std::string nsid1("server0");

        /******************************************************************/
        /* begin tests                                                    */
        /******************************************************************/
        // cp -r ns0://file0.txt 
        //    -> ns1:// = ns1://file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from SRC namespace's root to "
             "another NORNS_REMOTE_PATH at DST namespace's root "
             "(keeping the name)") {

            norns_iotask_t task =
                NORNS_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0.c_str(), 
                                              ctx.src_file_at_root.c_str()),
                             NORNS_REMOTE_PATH(nsid1.c_str(),
                                               remote_host.c_str(),
                                               ctx.dst_file_at_root0.c_str()));

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

                    }
                }
            }
        }

        // cp -r ns0://file0.txt 
        //    -> ns1://file1.txt = ns1://file1.txt
        WHEN("copying a single NORNS_LOCAL_PATH from SRC namespace's root to "
             "another NORNS_LOCAL_PATH at DST namespace's root "
             "(changing the name)") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0.c_str(), 
                                              ctx.src_file_at_root.c_str()),
                             NORNS_REMOTE_PATH(nsid1.c_str(), 
                                               remote_host.c_str(),
                                               ctx.dst_file_at_root1.c_str()));

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

                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../d/file0.txt 
        //    -> ns1://file0.txt = ns1://file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a SRC namespace's subdir "
             "to another NORNS_LOCAL_PATH at DST namespace's root") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0.c_str(), 
                                              ctx.src_file_at_subdir.c_str()),
                             NORNS_REMOTE_PATH(nsid1.c_str(),
                                               remote_host.c_str(),
                                               ctx.dst_file_at_root2.c_str()));

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

                    }

                }
            }
        }

        // cp -r ns0://a/b/c/.../d/file0.txt 
        //    -> ns1://file1.txt = ns1://file1.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a SRC namespace's subdir "
             "to another NORNS_LOCAL_PATH at DST namespace's root") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0.c_str(), 
                                              ctx.src_file_at_subdir.c_str()),
                             NORNS_REMOTE_PATH(nsid1.c_str(), 
                                               remote_host.c_str(), 
                                               ctx.dst_file_at_root3.c_str()));

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

                    }
                }
            }
        }

        // cp -r ns0://file0.txt 
        //    -> ns1://a/b/c/.../file0.txt = ns1://a/b/c/.../file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from SRC namespace's root to "
             "another NORNS_LOCAL_PATH at a DST namespace's subdir "
             "(keeping the name)") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0.c_str(), 
                                              ctx.src_file_at_root.c_str()),
                             NORNS_REMOTE_PATH(nsid1.c_str(), 
                                               remote_host.c_str(), 
                                               ctx.dst_file_at_subdir0.c_str()));

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

                    }
                }
            }
        }

        // cp -r ns0://file0.txt 
        //    -> ns1://a/b/c/.../file1.txt = ns1://a/b/c/.../file1.txt
        WHEN("copying a single NORNS_LOCAL_PATH from SRC namespace's root to "
             "another NORNS_LOCAL_PATH at a DST namespace's subdir "
             "(changing the name)") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0.c_str(), 
                                              ctx.src_file_at_root.c_str()),
                             NORNS_REMOTE_PATH(nsid1.c_str(), 
                                               remote_host.c_str(),
                                               ctx.dst_file_at_subdir1.c_str()));

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

                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../file0.txt 
        //    -> ns1://a/b/c/.../file0.txt = ns1://a/b/c/.../file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a SRC namespace's subdir "
             "to another NORNS_LOCAL_PATH at a DST namespace's subdir") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0.c_str(), 
                                              ctx.src_file_at_subdir.c_str()),
                             NORNS_REMOTE_PATH(nsid1.c_str(), 
                                               remote_host.c_str(),
                                               ctx.dst_file_at_subdir4.c_str()));

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

                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../file0.txt 
        //    -> ns1://a/b/c/.../file1.txt = ns1://a/b/c/.../file1.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a SRC namespace's subdir "
             "to another NORNS_LOCAL_PATH at a DST namespace's subdir") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0.c_str(), 
                                              ctx.src_file_at_subdir.c_str()),
                             NORNS_REMOTE_PATH(nsid1.c_str(), 
                                               remote_host.c_str(), 
                                               ctx.dst_file_at_subdir5.c_str()));

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

                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../file0.txt 
        //    -> ns1://e/f/g/.../file0.txt = ns1://e/f/g/.../file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a SRC namespace's subdir "
             "to another NORNS_LOCAL_PATH at a DST namespace's subdir "
             "(changing the parents names)") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0.c_str(), 
                                              ctx.src_file_at_subdir.c_str()),
                             NORNS_REMOTE_PATH(nsid1.c_str(), 
                                               remote_host.c_str(),
                                               ctx.dst_file_at_subdir2.c_str()));

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

                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../file0.txt 
        //    -> ns1://e/f/g/.../file1.txt = ns1://e/f/g/.../file1.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a SRC namespace's subdir "
             "to another NORNS_LOCAL_PATH at a DST namespace's subdir "
             "(changing the name)") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0.c_str(), 
                                              ctx.src_file_at_subdir.c_str()),
                             NORNS_REMOTE_PATH(nsid1.c_str(), 
                                               remote_host.c_str(), 
                                               ctx.dst_file_at_subdir3.c_str()));

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

                    }
                }
            }
        }








        env.notify_success();
    }
}
