/************************************************************************* 
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of NORNS, a service that allows other programs to   *
 * start, track and manage asynchronous transfers of data resources      *
 * between different storage backends.                                   *
 *                                                                       *
 * See AUTHORS file in the top level directory for information regarding *
 * developers and contributors.                                          *
 *                                                                       *
 * This software was developed as part of the EC H2020 funded project    *
 * NEXTGenIO (Project ID: 671951).                                       *
 *     www.nextgenio.eu                                                  *
 *                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining *
 * a copy of this software and associated documentation files (the       *
 * "Software"), to deal in the Software without restriction, including   *
 * without limitation the rights to use, copy, modify, merge, publish,   *
 * distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to *
 * the following conditions:                                             *
 *                                                                       *
 * The above copyright notice and this permission notice shall be        *
 * included in all copies or substantial portions of the Software.       *
 *                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                 *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS   *
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN    *
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN     *
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE      *
 * SOFTWARE.                                                             *
 *************************************************************************/

#include "nornsctl.h"
#include "test-env.hpp"
#include "compare-files.hpp"
#include "catch.hpp"

namespace bfs = boost::filesystem;

/******************************************************************************/
/* tests for push transfers (errors)                                          */
/******************************************************************************/
SCENARIO("errors copying local POSIX path to remote POSIX path (admin)", 
         "[api::nornsctl_submit_push_errors]") {
    GIVEN("a running urd instance") {

        /**********************************************************************/
        /* setup common environment                                           */
        /**********************************************************************/
        test_env env(false);

        const char* nsid0 = "tmp0";
        const char* nsid1 = "tmp1";
        const char* remote_host = "127.0.0.1:42000";
        bfs::path src_mnt, dst_mnt;

        // create namespaces
        std::tie(std::ignore, src_mnt) = 
            env.create_namespace(nsid0, "mnt/tmp0", 16384);
        std::tie(std::ignore, dst_mnt) = 
            env.create_namespace(nsid1, "mnt/tmp1", 16384);

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

        // create input data
        env.add_to_namespace(nsid0, src_file_at_root, 40000);
        env.add_to_namespace(nsid0, src_file_at_subdir, 80000);
        env.add_to_namespace(nsid0, src_subdir0);
        env.add_to_namespace(nsid0, src_subdir1);
        env.add_to_namespace(nsid0, src_empty_dir);

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir0 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir1 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        // create input data with special permissions
        auto p = env.add_to_namespace(nsid0, src_noperms_file0, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file1, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file2, 0);
        env.remove_access(p.parent_path());

        p = env.add_to_namespace(nsid0, src_noperms_subdir0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir1);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir2);
        env.remove_access(p.parent_path());

        // add symlinks to the namespace
        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_root0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_root1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_root2);

        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_subdir0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_subdir1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_subdir2);

        // manually create a symlink leading outside namespace 0
        boost::system::error_code ec;
        const bfs::path out_symlink = "/out_symlink";
        bfs::create_symlink(dst_mnt, src_mnt / out_symlink, ec);
        REQUIRE(!ec);

        // create required output directories
        env.add_to_namespace(nsid1, dst_subdir1);

        /**********************************************************************/
        /* begin tests                                                        */
        /**********************************************************************/
        // - trying to copy a non-existing file
        WHEN("copying a non-existing NORNS_LOCAL_PATH file") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_invalid_file.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and ENOENT are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(stats.st_sys_errno == ENOENT);
                    }
                }
            }
        }

        // - trying to copy a non-existing directory
        WHEN("copying a non-existing NORNS_LOCAL_PATH directory") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_invalid_dir.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and ENOENT are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(stats.st_sys_errno == ENOENT);
                    }
                }
            }
        }

        // - trying to copy an empty directory
        WHEN("copying an empty NORNS_LOCAL_PATH directory") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_empty_dir.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_SUCCESS and ENOENT are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);
                        REQUIRE(stats.st_task_error == NORNS_SUCCESS);
                        REQUIRE(stats.st_sys_errno == 0);

                        REQUIRE(bfs::exists(dst_mnt / dst_file_at_root0));
                    }
                }
            }
        }

//FIXME: DISABLED in CI until impersonation is implemented or capabilities can be added to the docker service
#ifdef __SETCAP_TESTS__

        // - trying to copy a file from namespace root with invalid access permissions
        WHEN("copying a NORNS_LOCAL_PATH file from \"/\" without appropriate "
             "permissions to access it") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_noperms_file0.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and EACCES|EPERM|EINVAL "
                         "are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(( (stats.st_sys_errno == EACCES) ||
                                  (stats.st_sys_errno == EPERM ) ||
                                  (stats.st_sys_errno == EINVAL) ));
                    }
                }
            }
        }

        // - trying to copy a file from namespace root with invalid access permissions
        WHEN("copying a NORNS_LOCAL_PATH file from a subdir without "
             "appropriate permissions to access it") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_noperms_file1.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and EACCES|EPERM|EINVAL are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(( (stats.st_sys_errno == EACCES) ||
                                  (stats.st_sys_errno == EPERM ) ||
                                  (stats.st_sys_errno == EINVAL) ));
                    }
                }
            }
        }

        // - trying to copy a file from namespace root with invalid access permissions
        WHEN("copying a NORNS_LOCAL_PATH file from a subdir without "
             "appropriate permissions to access a parent") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_noperms_file2.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and EACCES|EPERM|EINVAL "
                         "are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(( (stats.st_sys_errno == EACCES) ||
                                  (stats.st_sys_errno == EPERM ) ||
                                  (stats.st_sys_errno == EINVAL) ));
                    }
                }
            }
        }

        // - trying to copy a subdir from namespace root with invalid access permissions
        WHEN("copying a NORNS_LOCAL_PATH subdir from \"/\"  without "
             "appropriate permissions to access it") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_noperms_subdir0.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and EACCES|EPERM|EINVAL "
                         "are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(( (stats.st_sys_errno == EACCES) ||
                                  (stats.st_sys_errno == EPERM ) ||
                                  (stats.st_sys_errno == EINVAL) ));
                    }
                }
            }
        }

        // - trying to copy a subdir from namespace root with invalid access permissions
        WHEN("copying a NORNS_LOCAL_PATH subdir from another subdir "
             "without appropriate permissions to access it") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_noperms_subdir1.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and EACCES|EPERM|EINVAL are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(( (stats.st_sys_errno == EACCES) ||
                                  (stats.st_sys_errno == EPERM ) ||
                                  (stats.st_sys_errno == EINVAL) ));
                    }
                }
            }
        }

        // - trying to copy a subdir from namespace root with invalid access permissions
        WHEN("copying a NORNS_LOCAL_PATH subdir from another subdir without "
             "appropriate permissions to access a parent") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_noperms_subdir2.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and EACCES|EPERM|EINVAL "
                         "are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(( (stats.st_sys_errno == EACCES) ||
                                  (stats.st_sys_errno == EPERM ) ||
                                  (stats.st_sys_errno == EINVAL) ));
                    }
                }
            }
        }

        // symlink leading out of namespace
        WHEN("copying a NORNS_LOCAL_PATH through a symbolic link that leads "
             "out of the SRC namespace") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              out_symlink.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and ENOENT are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(stats.st_sys_errno == ENOENT);
                    }
                }
            }
        }
#endif

        env.notify_success();
    }
}

/******************************************************************************/
/* tests for push transfers (single files)                                    */
/******************************************************************************/
SCENARIO("copy local POSIX file to remote POSIX file (admin)", 
         "[api::nornsctl_submit_push_to_posix_file]") {
    GIVEN("a running urd instance") {

        /**********************************************************************/
        /* setup common environment                                           */
        /**********************************************************************/
        test_env env(false);

        const char* nsid0 = "tmp0";
        const char* nsid1 = "tmp1";
        const char* remote_host = "127.0.0.1:42000";
        bfs::path src_mnt, dst_mnt;

        // create namespaces
        std::tie(std::ignore, src_mnt) = 
            env.create_namespace(nsid0, "mnt/tmp0", 16384);
        std::tie(std::ignore, dst_mnt) = 
            env.create_namespace(nsid1, "mnt/tmp1", 16384);

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

        // create input data
        env.add_to_namespace(nsid0, src_file_at_root, 40000);
        env.add_to_namespace(nsid0, src_file_at_subdir, 80000);
        env.add_to_namespace(nsid0, src_subdir0);
        env.add_to_namespace(nsid0, src_subdir1);
        env.add_to_namespace(nsid0, src_empty_dir);

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir0 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir1 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        // create input data with special permissions
        auto p = env.add_to_namespace(nsid0, src_noperms_file0, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file1, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file2, 0);
        env.remove_access(p.parent_path());

        p = env.add_to_namespace(nsid0, src_noperms_subdir0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir1);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir2);
        env.remove_access(p.parent_path());

        // add symlinks to the namespace
        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_root0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_root1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_root2);

        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_subdir0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_subdir1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_subdir2);

        // manually create a symlink leading outside namespace 0
        boost::system::error_code ec;
        const bfs::path out_symlink = "/out_symlink";
        bfs::create_symlink(dst_mnt, src_mnt / out_symlink, ec);
        REQUIRE(!ec);

        // create required output directories
        env.add_to_namespace(nsid1, dst_subdir1);


        /**********************************************************************/
        /* begin tests                                                        */
        /**********************************************************************/
        // cp -r ns0://file0.txt 
        //    -> ns1:// = ns1://file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from SRC namespace's root to "
             "another NORNS_REMOTE_PATH at DST namespace's root "
             "(keeping the name)") {

            norns_iotask_t task =
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_file_at_root.c_str()),
                             NORNS_REMOTE_PATH(nsid1,
                                               remote_host,
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {

                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_root);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_root0);

                            REQUIRE(compare_files(src, dst) == true);
                        }
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
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_file_at_root.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host,
                                               dst_file_at_root1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_root);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_root1);

                            REQUIRE(compare_files(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../d/file0.txt 
        //    -> ns1://file0.txt = ns1://file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a SRC namespace's subdir "
             "to another NORNS_LOCAL_PATH at DST namespace's root (keeping "
             "the name)") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_file_at_subdir.c_str()),
                             NORNS_REMOTE_PATH(nsid1,
                                               remote_host,
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_subdir);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_root0);

                            REQUIRE(compare_files(src, dst) == true);
                        }
                    }

                }
            }
        }

        // cp -r ns0://a/b/c/.../d/file0.txt 
        //    -> ns1://file1.txt = ns1://file1.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a SRC namespace's subdir "
             "to another NORNS_LOCAL_PATH at DST namespace's root (changing "
             "the name)") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_file_at_subdir.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_subdir);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_root1);

                            REQUIRE(compare_files(src, dst) == true);
                        }
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
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_file_at_root.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_subdir0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_root);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_subdir0);

                            REQUIRE(compare_files(src, dst) == true);
                        }
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
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_file_at_root.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host,
                                               dst_file_at_subdir1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_root);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_subdir1);

                            REQUIRE(compare_files(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../file0.txt 
        //    -> ns1://a/b/c/.../file0.txt = ns1://a/b/c/.../file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a SRC namespace's subdir "
             "to another NORNS_LOCAL_PATH at a DST namespace's subdir "
             "(keeping the name)") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_file_at_subdir.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host,
                                               dst_file_at_subdir0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_subdir);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_subdir0);

                            REQUIRE(compare_files(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../file0.txt 
        //    -> ns1://a/b/c/.../file1.txt = ns1://a/b/c/.../file1.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a SRC namespace's subdir "
             "to another NORNS_LOCAL_PATH at a DST namespace's subdir "
             "(changing the name)") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_file_at_subdir.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_subdir1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_subdir);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_subdir1);

                            REQUIRE(compare_files(src, dst) == true);
                        }
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
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_file_at_subdir.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host,
                                               dst_file_at_subdir2.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_subdir);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_subdir2);

                            REQUIRE(compare_files(src, dst) == true);
                        }
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
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_file_at_subdir.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_subdir3.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_subdir);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_subdir3);

                            REQUIRE(compare_files(src, dst) == true);
                        }
                    }
                }
            }
        }

        env.notify_success();
    }
}

/******************************************************************************/
/* tests for push transfers (directories)                                     */
/******************************************************************************/
SCENARIO("copy local POSIX file to remote POSIX subdir (admin)", 
         "[api::nornsctl_submit_push_to_posix_subdir]") {
    GIVEN("a running urd instance") {

        /**********************************************************************/
        /* setup common environment                                           */
        /**********************************************************************/
        test_env env(false);

        const char* nsid0 = "tmp0";
        const char* nsid1 = "tmp1";
        const char* remote_host = "127.0.0.1:42000";
        bfs::path src_mnt, dst_mnt;

        // create namespaces
        std::tie(std::ignore, src_mnt) = 
            env.create_namespace(nsid0, "mnt/tmp0", 16384);
        std::tie(std::ignore, dst_mnt) = 
            env.create_namespace(nsid1, "mnt/tmp1", 16384);

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

        // create input data
        env.add_to_namespace(nsid0, src_file_at_root, 40000);
        env.add_to_namespace(nsid0, src_file_at_subdir, 80000);
        env.add_to_namespace(nsid0, src_subdir0);
        env.add_to_namespace(nsid0, src_subdir1);
        env.add_to_namespace(nsid0, src_empty_dir);

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir0 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir1 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        // create input data with special permissions
        auto p = env.add_to_namespace(nsid0, src_noperms_file0, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file1, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file2, 0);
        env.remove_access(p.parent_path());

        p = env.add_to_namespace(nsid0, src_noperms_subdir0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir1);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir2);
        env.remove_access(p.parent_path());

        // add symlinks to the namespace
        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_root0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_root1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_root2);

        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_subdir0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_subdir1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_subdir2);

        // manually create a symlink leading outside namespace 0
        boost::system::error_code ec;
        const bfs::path out_symlink = "/out_symlink";
        bfs::create_symlink(dst_mnt, src_mnt / out_symlink, ec);
        REQUIRE(!ec);


        // create required output directories
        env.add_to_namespace(nsid1, dst_subdir1);

        /**********************************************************************/
        /* begin tests                                                        */
        /**********************************************************************/
        // cp -r /a/contents.* -> / = /contents.*
        WHEN("copying the contents of a NORNS_LOCAL_PATH subdir from SRC "
             "namespace's root to DST namespace's root\n"
             "  cp -r /a/contents.* -> / = /contents.* ") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_subdir0.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_root.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Copied files are identical to original") {
                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_subdir0);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_root);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r /a/b/c/.../contents.* -> / = /contents.*
        WHEN("copying the contents of a NORNS_LOCAL_PATH arbitrary subdir to "
             "DST namespace's root\n"
             "  cp -r /a/b/c/.../contents.* -> / = /contents.*") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_subdir1.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_root.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Copied files are identical to original") {
                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_subdir1);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_root);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r /a/contents.* -> /c = /c/contents.*
        // (c did not exist previously)
        WHEN("copying the contents of a NORNS_LOCAL_PATH subdir from SRC "
             "namespace's root to another NORNS_REMOTE_PATH subdir at DST "
             "namespace's root while changing its name\n"
             "  cp -r /a/contents.* -> /c = /c/contents.*") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_subdir0.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_subdir0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Copied files are identical to original") {
                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_subdir0);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_subdir0);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r /a/contents.* -> /c = /c/contents.*
        // (c did exist previously)
        WHEN("copying the contents of a NORNS_LOCAL_PATH subdir from SRC "
             "namespace's root to another NORNS_REMOTE_PATH subdir at DST "
             "namespace's root while changing its name\n"
             "  cp -r /a/contents.* -> /c / /c/contents.*") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_subdir0.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_subdir1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Copied files are identical to original") {
                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_subdir0);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_subdir1);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r /a/b/c/.../contents.* -> /c = /c/a/b/c/.../contents.*
        // (c did not exist previously)
        WHEN("copying the contents of a NORNS_LOCAL_PATH subdir from SRC "
             "namespace's root to a NORNS_REMOTE_PATH subdir at DST "
             "namespace's root while changing its name\n"
             "cp -r /a/b/c/.../contents.* -> /c = /c/a/b/c/.../contents.*") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_subdir1.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_subdir0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Copied files are identical to original") {
                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_subdir1);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_subdir0);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r /a/b/c/.../contents.* -> /c = /c/a/b/c/.../contents.*
        // (c did exist previously)
        WHEN("copying the contents of a NORNS_LOCAL_PATH subdir from SRC "
             "namespace's root to another NORNS_REMOTE_PATH subdir at DST "
             "namespace's root while changing its name:"
             "  cp -r /a/b/c/.../contents.* -> /c = /c/a/b/c/.../contents.*") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_subdir1.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_subdir1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Copied files are identical to original") {
                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_subdir1);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_subdir1);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        env.notify_success();
    }
}

/******************************************************************************/
/* tests for push transfers (memory buffers)                                  */
/******************************************************************************/
SCENARIO("copy local memory region to remote POSIX file (admin)", 
         "[api::nornsctl_submit_push_memory_to_posix_file]") {

    GIVEN("a running urd instance") {

        /**********************************************************************/
        /* setup common environment                                           */
        /**********************************************************************/
        test_env env;

        const char* nsid0 = "tmp0";
        const char* remote_host = "127.0.0.1:42000";
        bfs::path dst_mnt;

        // create namespaces
        std::tie(std::ignore, dst_mnt) = env.create_namespace(nsid0, "mnt/tmp0", 16384);

        // create input data buffer (around 40MiBs)
        std::vector<int> input_data(10000000, 42);
        void* region_addr = input_data.data();
        size_t region_size = input_data.size() * sizeof(int);

        // output names
        const bfs::path dst_file_at_root0   = "/file0";
        const bfs::path dst_file_at_subdir0 = "/a/b/c/d/file0";
        const bfs::path dst_root            = "/";
        const bfs::path dst_subdir0         = "/output_dir0/"; // existing
        const bfs::path dst_subdir1         = "/output_dir0"; // existing but does not look as a directory
        const bfs::path dst_subdir2         = "/output_dir0/a/b/c/d/"; // existing
        const bfs::path dst_subdir3         = "/output_dir0/a/b/c/d"; // existing but does not look as a directory
        const bfs::path dst_subdir4         = "/output_dir1/"; // non-existing
        const bfs::path dst_subdir5         = "/output_dir1/a/b/c/d/"; // non-existing

        // create required output directories
        env.add_to_namespace(nsid0, dst_subdir0);
        env.add_to_namespace(nsid0, dst_subdir2);

        // 
        WHEN("copying a valid memory region to a NORNS_REMOTE_PATH file "
             "located at DST's namespace root '/'") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_MEMORY_REGION(region_addr, region_size), 
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host,
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() return NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Output file contains buffer data") {

                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid0, dst_file_at_root0);

                            REQUIRE(compare(input_data, dst) == true);
                        }
                    }
                }
            }
        }

        WHEN("copying a valid memory region to a NORNS_REMOTE_PATH file "
             "located at a DST's subdir") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_MEMORY_REGION(region_addr, region_size), 
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host,
                                               dst_file_at_subdir0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_error() reports NORNS_EFINISHED") {
                    norns_stat_t stats;
                    rv = nornsctl_error(&task, &stats);

                    REQUIRE(rv == NORNS_SUCCESS);
                    REQUIRE(stats.st_status == NORNS_EFINISHED);

                    THEN("nornsctl_wait() return NORNS_SUCCESS") {
                        REQUIRE(rv == NORNS_SUCCESS);

                        THEN("Output file contains buffer data") {

                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid0, dst_file_at_subdir0);

                            REQUIRE(compare(input_data, dst) == true);
                        }
                    }
                }
            }
        }

        env.notify_success();
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("attempting to request a transfer") {

            norns_iotask_t task = NORNSCTL_IOTASK(
                NORNS_IOTASK_COPY,
                NORNS_LOCAL_PATH("nvml0://", "/a/b/c/"),
                NORNS_REMOTE_PATH("nvml0://", "node1", "/a/b/d/"));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
#endif
}


/******************************************************************************/
/* tests for push transfers (memory buffers, errors)                          */
/******************************************************************************/
SCENARIO("errors copying local memory region to remote POSIX file (admin)", 
         "[api::nornsctl_submit_push_memory_to_posix_file_errors]") {

    GIVEN("a running urd instance") {

        /**********************************************************************/
        /* setup common environment                                           */
        /**********************************************************************/
        test_env env;

        const char* nsid0 = "tmp0";
        const char* remote_host = "127.0.0.1:42000";
        bfs::path dst_mnt;

        // create namespaces
        std::tie(std::ignore, dst_mnt) = env.create_namespace(nsid0, "mnt/tmp0", 16384);

        // create input data buffer (around 40MiBs)
        std::vector<int> input_data(10000000, 42);
        void* region_addr = input_data.data();
        size_t region_size = input_data.size() * sizeof(int);

        // output names
        const bfs::path dst_file_at_root0   = "/file0";
        const bfs::path dst_file_at_subdir0 = "/a/b/c/d/file0";
        const bfs::path dst_root            = "/";
        const bfs::path dst_subdir0         = "/output_dir0/"; // existing
        const bfs::path dst_subdir1         = "/output_dir0"; // existing but does not look as a directory
        const bfs::path dst_subdir2         = "/output_dir0/a/b/c/d/"; // existing
        const bfs::path dst_subdir3         = "/output_dir0/a/b/c/d"; // existing but does not look as a directory
        const bfs::path dst_subdir4         = "/output_dir1/"; // non-existing
        const bfs::path dst_subdir5         = "/output_dir1/a/b/c/d/"; // non-existing

        // create required output directories
        env.add_to_namespace(nsid0, dst_subdir0);
        env.add_to_namespace(nsid0, dst_subdir2);

        //TODO
        // - copy a valid but removed memory region (cannot control => undefined behavior)
        // - providing a non-existing directory path (i.e. finished with /) as output name
        // - providing an existing path that points to a directory as output name
        WHEN("copying an invalid memory region to a NORNS_REMOTE_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_MEMORY_REGION((void*) 0x42, 42000),
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host,
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() return NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_ESYSTEMERROR and "
                         "EFAULT") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(stats.st_sys_errno == EFAULT);
                    }
                }
            }
        }

        WHEN("copying a valid memory region to a NORNS_REMOTE_PATH at "
             "DST's /") {

            // create input data buffer
            std::vector<int> input_data(100, 42);
            void* region_addr = input_data.data();
            size_t region_size = input_data.size() * sizeof(int);

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_MEMORY_REGION(region_addr, region_size),
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               dst_root.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_EBADARGS") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("copying a valid memory region to a NORNS_REMOTE_PATH existing "
             "directory at DST's /") {

            // create input data buffer
            std::vector<int> input_data(100, 42);
            void* region_addr = input_data.data();
            size_t region_size = input_data.size() * sizeof(int);

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_MEMORY_REGION(region_addr, region_size),
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               dst_subdir0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_EBADARGS") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("copying a valid memory region to a NORNS_REMOTE_PATH existing "
             "directory at / that does not look like a directory") {

            // create input data buffer
            std::vector<int> input_data(100, 42);
            void* region_addr = input_data.data();
            size_t region_size = input_data.size() * sizeof(int);

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_MEMORY_REGION(region_addr, region_size),
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               dst_subdir1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() return NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_ESYSTEMERROR and "
                         "EISDIR") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(stats.st_sys_errno == EISDIR);
                    }
                }
            }
        }

        WHEN("copying a valid memory region to a NORNS_REMOTE_PATH existing "
             "directory at a DST's arbitary subdir") {

            // create input data buffer
            std::vector<int> input_data(100, 42);
            void* region_addr = input_data.data();
            size_t region_size = input_data.size() * sizeof(int);

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_MEMORY_REGION(region_addr, region_size),
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               dst_subdir2.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_EBADARGS") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("copying a valid memory region to a NORNS_REMOTE_PATH existing "
             "directory at a DST's arbitary subdir that does not look like "
             "a directory") {

            // create input data buffer
            std::vector<int> input_data(100, 42);
            void* region_addr = input_data.data();
            size_t region_size = input_data.size() * sizeof(int);

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_MEMORY_REGION(region_addr, region_size),
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               dst_subdir3.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() return NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_ESYSTEMERROR and "
                         "EISDIR") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(stats.st_sys_errno == EISDIR);
                    }
                }
            }
        }


        // i.e. a destination path that 'looks like' a directory
        WHEN("copying a valid memory region to a NORNS_REMOTE_PATH "
             "corresponding to a non-existing directory at DST's /") {

            // create input data buffer
            std::vector<int> input_data(100, 42);
            void* region_addr = input_data.data();
            size_t region_size = input_data.size() * sizeof(int);

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_MEMORY_REGION(region_addr, region_size),
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               dst_subdir4.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_EBADARGS") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("copying a valid memory region to a NORNS_REMOTE_PATH "
             "non-existing arbitrary subdir at DST") {

            // create input data buffer
            std::vector<int> input_data(100, 42);
            void* region_addr = input_data.data();
            size_t region_size = input_data.size() * sizeof(int);

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_MEMORY_REGION(region_addr, region_size),
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host,
                                               dst_subdir5.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_EBADARGS") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }


        env.notify_success();
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("attempting to request a transfer") {

            norns_iotask_t task = NORNSCTL_IOTASK(
                NORNS_IOTASK_COPY,
                NORNS_LOCAL_PATH("nvml0://", "/a/b/c/"),
                NORNS_REMOTE_PATH("nvml0://", "node1", "/a/b/d/"));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
#endif
}

/******************************************************************************/
/* tests for push transfers (links)                                           */
/******************************************************************************/
SCENARIO("copy local POSIX path to remote POSIX path involving links (admin)", 
         "[api::nornsctl_submit_push_links]") {
    GIVEN("a running urd instance") {

        /**********************************************************************/
        /* setup common environment                                           */
        /**********************************************************************/
        test_env env(false);

        const char* nsid0 = "tmp0";
        const char* nsid1 = "tmp1";
        const char* remote_host = "127.0.0.1:42000";
        bfs::path src_mnt, dst_mnt;

        // create namespaces
        std::tie(std::ignore, src_mnt) = 
            env.create_namespace(nsid0, "mnt/tmp0", 16384);
        std::tie(std::ignore, dst_mnt) = 
            env.create_namespace(nsid1, "mnt/tmp1", 16384);

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

        // create input data
        env.add_to_namespace(nsid0, src_file_at_root, 40000);
        env.add_to_namespace(nsid0, src_file_at_subdir, 80000);
        env.add_to_namespace(nsid0, src_subdir0);
        env.add_to_namespace(nsid0, src_subdir1);
        env.add_to_namespace(nsid0, src_empty_dir);

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir0 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir1 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        // create input data with special permissions
        auto p = env.add_to_namespace(nsid0, src_noperms_file0, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file1, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file2, 0);
        env.remove_access(p.parent_path());

        p = env.add_to_namespace(nsid0, src_noperms_subdir0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir1);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir2);
        env.remove_access(p.parent_path());

        // add symlinks to the namespace
        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_root0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_root1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_root2);

        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_subdir0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_subdir1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_subdir2);

        // manually create a symlink leading outside namespace 0
        boost::system::error_code ec;
        const bfs::path out_symlink = "/out_symlink";
        bfs::create_symlink(dst_mnt, src_mnt / out_symlink, ec);
        REQUIRE(!ec);


        // create required output directories
        env.add_to_namespace(nsid1, dst_subdir1);

        /**********************************************************************/
        /* begin tests                                                        */
        /**********************************************************************/
        WHEN("copying a single NORNS_LOCAL_PATH file from SRC namespace's '/' "
             "through a symlink also located at '/'" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_symlink_at_root0.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {

                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_symlink_at_root0);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_file_at_root0);

                            REQUIRE(compare_files(src, dst) == true);
                        }
                    }
                }
            }
        }

        WHEN("copying a single NORNS_LOCAL_PATH subdir from SRC "
             "namespace's '/' through a symlink also located at '/'" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_symlink_at_root1.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Directories are equal") {

                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_symlink_at_root1);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_file_at_root0);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        WHEN("copying a single NORNS_LOCAL_PATH arbitrary subdir"
             "through a symlink also located at '/'" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_symlink_at_root2.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host,
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Directories are equal") {

                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_symlink_at_root2);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_file_at_root0);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        WHEN("copying a single NORNS_LOCAL_PATH file from SRC namespace's '/' "
             "through a symlink located in a subdir" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_symlink_at_subdir0.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {

                            bfs::path src = 
                                env.get_from_namespace(nsid0, 
                                                    src_symlink_at_subdir0);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_file_at_root0);

                            REQUIRE(compare_files(src, dst) == true);
                        }
                    }
                }
            }
        }

        WHEN("copying a single NORNS_LOCAL_PATH subdir from SRC "
             "namespace's '/' through a symlink also located at subdir" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_symlink_at_subdir1.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Directories are equal") {

                            bfs::path src = 
                                env.get_from_namespace(nsid0, 
                                                    src_symlink_at_subdir1);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_file_at_root0);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        WHEN("copying a single NORNS_LOCAL_PATH arbitrary subdir"
             "through a symlink also located at a subdir" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_symlink_at_subdir2.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host,
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Directories are equal") {

                            bfs::path src = 
                                env.get_from_namespace(nsid0, 
                                                    src_symlink_at_subdir2);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_file_at_root0);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        env.notify_success();
    }
}

/******************************************************************************/
/* tests for pull transfers (errors)                                          */
/******************************************************************************/
SCENARIO("errors copying remote POSIX path to local POSIX path (admin)", 
         "[api::nornsctl_submit_pull_errors]") {
    GIVEN("a running urd instance") {

        /**********************************************************************/
        /* setup common environment                                           */
        /**********************************************************************/
        test_env env(false);

        const char* nsid0 = "tmp0";
        const char* nsid1 = "tmp1";
        const char* remote_host = "127.0.0.1:42000";
        bfs::path src_mnt, dst_mnt;

        // create namespaces
        std::tie(std::ignore, src_mnt) = 
            env.create_namespace(nsid0, "mnt/tmp0", 16384);
        std::tie(std::ignore, dst_mnt) = 
            env.create_namespace(nsid1, "mnt/tmp1", 16384);

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

        // create input data
        env.add_to_namespace(nsid0, src_file_at_root, 40000);
        env.add_to_namespace(nsid0, src_file_at_subdir, 80000);
        env.add_to_namespace(nsid0, src_subdir0);
        env.add_to_namespace(nsid0, src_subdir1);
        env.add_to_namespace(nsid0, src_empty_dir);

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir0 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir1 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        // create input data with special permissions
        auto p = env.add_to_namespace(nsid0, src_noperms_file0, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file1, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file2, 0);
        env.remove_access(p.parent_path());

        p = env.add_to_namespace(nsid0, src_noperms_subdir0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir1);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir2);
        env.remove_access(p.parent_path());

        // add symlinks to the namespace
        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_root0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_root1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_root2);

        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_subdir0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_subdir1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_subdir2);

        // manually create a symlink leading outside namespace 0
        boost::system::error_code ec;
        const bfs::path out_symlink = "/out_symlink";
        bfs::create_symlink(dst_mnt, src_mnt / out_symlink, ec);
        REQUIRE(!ec);

        // create required output directories
        env.add_to_namespace(nsid1, dst_subdir1);

        /**********************************************************************/
        /* begin tests                                                        */
        /**********************************************************************/
        // - trying to copy a non-existing file
        WHEN("copying a non-existing NORNS_LOCAL_PATH file") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               src_invalid_file.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and ENOENT are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(stats.st_sys_errno == ENOENT);
                    }
                }
            }
        }

        // - trying to copy a non-existing directory
        WHEN("copying a non-existing NORNS_LOCAL_PATH directory") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               src_invalid_dir.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and ENOENT are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(stats.st_sys_errno == ENOENT);
                    }
                }
            }
        }

        // - trying to copy an empty directory
        WHEN("copying an empty NORNS_LOCAL_PATH directory") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                              remote_host, 
                                              src_empty_dir.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_SUCCESS and ENOENT are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);
                        REQUIRE(stats.st_task_error == NORNS_SUCCESS);
                        REQUIRE(stats.st_sys_errno == 0);

                        REQUIRE(bfs::exists(dst_mnt / dst_file_at_root0));
                    }
                }
            }
        }

//FIXME: DISABLED in CI until impersonation is implemented or capabilities can be added to the docker service
#ifdef __SETCAP_TESTS__

#if 0 // not adapted to pull semantics
        // - trying to copy a file from namespace root with invalid access permissions
        WHEN("copying a NORNS_LOCAL_PATH file from \"/\" without appropriate "
             "permissions to access it") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_noperms_file0.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and EACCES|EPERM|EINVAL "
                         "are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(( (stats.st_sys_errno == EACCES) ||
                                  (stats.st_sys_errno == EPERM ) ||
                                  (stats.st_sys_errno == EINVAL) ));
                    }
                }
            }
        }

        // - trying to copy a file from namespace root with invalid access permissions
        WHEN("copying a NORNS_LOCAL_PATH file from a subdir without "
             "appropriate permissions to access it") {

            norns_op_t task_op = NORNS_IOTASK_COPY;

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_noperms_file1.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and EACCES|EPERM|EINVAL are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(( (stats.st_sys_errno == EACCES) ||
                                  (stats.st_sys_errno == EPERM ) ||
                                  (stats.st_sys_errno == EINVAL) ));
                    }
                }
            }
        }

        // - trying to copy a file from namespace root with invalid access permissions
        WHEN("copying a NORNS_LOCAL_PATH file from a subdir without "
             "appropriate permissions to access a parent") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, src_noperms_file2.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and EACCES|EPERM|EINVAL "
                         "are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(( (stats.st_sys_errno == EACCES) ||
                                  (stats.st_sys_errno == EPERM ) ||
                                  (stats.st_sys_errno == EINVAL) ));
                    }
                }
            }
        }

        // - trying to copy a subdir from namespace root with invalid access permissions
        WHEN("copying a NORNS_LOCAL_PATH subdir from \"/\"  without "
             "appropriate permissions to access it") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_noperms_subdir0.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and EACCES|EPERM|EINVAL "
                         "are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(( (stats.st_sys_errno == EACCES) ||
                                  (stats.st_sys_errno == EPERM ) ||
                                  (stats.st_sys_errno == EINVAL) ));
                    }
                }
            }
        }

        // - trying to copy a subdir from namespace root with invalid access permissions
        WHEN("copying a NORNS_LOCAL_PATH subdir from another subdir "
             "without appropriate permissions to access it") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_noperms_subdir1.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and EACCES|EPERM|EINVAL are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(( (stats.st_sys_errno == EACCES) ||
                                  (stats.st_sys_errno == EPERM ) ||
                                  (stats.st_sys_errno == EINVAL) ));
                    }
                }
            }
        }

        // - trying to copy a subdir from namespace root with invalid access permissions
        WHEN("copying a NORNS_LOCAL_PATH subdir from another subdir without "
             "appropriate permissions to access a parent") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_noperms_subdir2.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and EACCES|EPERM|EINVAL "
                         "are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(( (stats.st_sys_errno == EACCES) ||
                                  (stats.st_sys_errno == EPERM ) ||
                                  (stats.st_sys_errno == EINVAL) ));
                    }
                }
            }
        }

        // symlink leading out of namespace
        WHEN("copying a NORNS_LOCAL_PATH through a symbolic link that leads "
             "out of the SRC namespace") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_LOCAL_PATH(nsid0, 
                                              out_symlink.c_str()),
                             NORNS_REMOTE_PATH(nsid1, 
                                               remote_host, 
                                               dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("NORNS_ESYSTEMERROR and ENOENT are reported") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(stats.st_sys_errno == ENOENT);
                    }
                }
            }
        }
#endif
#endif
        env.notify_success();
    }
}

/******************************************************************************/
/* tests for pull transfers (single files)                                    */
/******************************************************************************/
SCENARIO("copy remote POSIX file to local POSIX file (admin)", 
         "[api::nornsctl_submit_pull_to_posix_file]") {
    GIVEN("a running urd instance") {

        /**********************************************************************/
        /* setup common environment                                           */
        /**********************************************************************/
        test_env env(false);

        const char* nsid0 = "tmp0";
        const char* nsid1 = "tmp1";
        const char* remote_host = "127.0.0.1:42000";
        bfs::path src_mnt, dst_mnt;

        // create namespaces
        std::tie(std::ignore, src_mnt) = 
            env.create_namespace(nsid0, "mnt/tmp0", 16384);
        std::tie(std::ignore, dst_mnt) = 
            env.create_namespace(nsid1, "mnt/tmp1", 16384);

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

        // create input data
        env.add_to_namespace(nsid0, src_file_at_root, 40000);
        env.add_to_namespace(nsid0, src_file_at_subdir, 80000);
        env.add_to_namespace(nsid0, src_subdir0);
        env.add_to_namespace(nsid0, src_subdir1);
        env.add_to_namespace(nsid0, src_empty_dir);

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir0 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir1 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        // create input data with special permissions
        auto p = env.add_to_namespace(nsid0, src_noperms_file0, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file1, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file2, 0);
        env.remove_access(p.parent_path());

        p = env.add_to_namespace(nsid0, src_noperms_subdir0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir1);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir2);
        env.remove_access(p.parent_path());

        // add symlinks to the namespace
        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_root0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_root1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_root2);

        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_subdir0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_subdir1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_subdir2);

        // manually create a symlink leading outside namespace 0
        boost::system::error_code ec;
        const bfs::path out_symlink = "/out_symlink";
        bfs::create_symlink(dst_mnt, src_mnt / out_symlink, ec);
        REQUIRE(!ec);


        // create required output directories
        env.add_to_namespace(nsid1, dst_subdir1);

        /**********************************************************************/
        /* begin tests                                                        */
        /**********************************************************************/
        // cp -r ns0://file0.txt 
        //    -> ns1:// = ns1://file0.txt
        WHEN("copying a single NORNS_REMOTE_PATH from SRC namespace's root to "
             "another NORNS_LOCAL_PATH at DST namespace's root "
             "(keeping the name)") {

            norns_iotask_t task =
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0,
                                               remote_host,
                                               src_file_at_root.c_str()),
                             NORNS_LOCAL_PATH(nsid1,
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() return NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {

                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_file_at_root);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_file_at_root0);

                            REQUIRE(compare_files(src, dst) == true);
                        }
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
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host,
                                               src_file_at_root.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() return NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_file_at_root);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_file_at_root1);

                            REQUIRE(compare_files(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../d/file0.txt 
        //    -> ns1://file0.txt = ns1://file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a SRC namespace's subdir "
             "to another NORNS_LOCAL_PATH at DST namespace's root (keeping "
             "the name)") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host,
                                               src_file_at_subdir.c_str()),
                             NORNS_LOCAL_PATH(nsid1,
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                    nsid0, src_file_at_subdir);
                            bfs::path dst = 
                                env.get_from_namespace(
                                    nsid1, dst_file_at_root0);

                            REQUIRE(compare_files(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../d/file0.txt 
        //    -> ns1://file1.txt = ns1://file1.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a SRC namespace's subdir "
             "to another NORNS_LOCAL_PATH at DST namespace's root (changing "
             "the name)") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               src_file_at_subdir.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_subdir);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_root1);

                            REQUIRE(compare_files(src, dst) == true);
                        }
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
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               src_file_at_root.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_subdir0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(nsid0, 
                                                       src_file_at_root);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, 
                                                       dst_file_at_subdir0);

                            REQUIRE(compare_files(src, dst) == true);
                        }
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
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host,
                                               src_file_at_root.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_subdir1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_root);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_subdir1);

                            REQUIRE(compare_files(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../file0.txt 
        //    -> ns1://a/b/c/.../file0.txt = ns1://a/b/c/.../file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a SRC namespace's subdir "
             "to another NORNS_LOCAL_PATH at a DST namespace's subdir "
             "(keeping the name)") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                              remote_host,
                                              src_file_at_subdir.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_subdir0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_subdir);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_subdir0);

                            REQUIRE(compare_files(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../file0.txt 
        //    -> ns1://a/b/c/.../file1.txt = ns1://a/b/c/.../file1.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a SRC namespace's subdir "
             "to another NORNS_LOCAL_PATH at a DST namespace's subdir "
             "(changing the name)") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               src_file_at_subdir.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_subdir1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_subdir);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_subdir1);

                            REQUIRE(compare_files(src, dst) == true);
                        }
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
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host,
                                               src_file_at_subdir.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_subdir2.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_subdir);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_subdir2);

                            REQUIRE(compare_files(src, dst) == true);
                        }
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
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               src_file_at_subdir.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_subdir3.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {
                            bfs::path src = 
                                env.get_from_namespace(
                                        nsid0, src_file_at_subdir);
                            bfs::path dst = 
                                env.get_from_namespace(
                                        nsid1, dst_file_at_subdir3);

                            REQUIRE(compare_files(src, dst) == true);
                        }
                    }
                }
            }
        }

        env.notify_success();
    }
}

/******************************************************************************/
/* tests for pull transfers (directories)                                     */
/******************************************************************************/
SCENARIO("copy remote POSIX subdir to local POSIX subdir (admin)", 
         "[api::nornsctl_submit_pull_to_posix_subdir]") {
    GIVEN("a running urd instance") {

        /**********************************************************************/
        /* setup common environment                                           */
        /**********************************************************************/
        test_env env(false);

        const char* nsid0 = "tmp0";
        const char* nsid1 = "tmp1";
        const char* remote_host = "127.0.0.1:42000";
        bfs::path src_mnt, dst_mnt;

        // create namespaces
        std::tie(std::ignore, src_mnt) = 
            env.create_namespace(nsid0, "mnt/tmp0", 16384);
        std::tie(std::ignore, dst_mnt) = 
            env.create_namespace(nsid1, "mnt/tmp1", 16384);

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

        // create input data
        env.add_to_namespace(nsid0, src_file_at_root, 40000);
        env.add_to_namespace(nsid0, src_file_at_subdir, 80000);
        env.add_to_namespace(nsid0, src_subdir0);
        env.add_to_namespace(nsid0, src_subdir1);
        env.add_to_namespace(nsid0, src_empty_dir);

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir0 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir1 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        // create input data with special permissions
        auto p = env.add_to_namespace(nsid0, src_noperms_file0, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file1, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file2, 0);
        env.remove_access(p.parent_path());

        p = env.add_to_namespace(nsid0, src_noperms_subdir0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir1);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir2);
        env.remove_access(p.parent_path());

        // add symlinks to the namespace
        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_root0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_root1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_root2);

        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_subdir0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_subdir1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_subdir2);

        // manually create a symlink leading outside namespace 0
        boost::system::error_code ec;
        const bfs::path out_symlink = "/out_symlink";
        bfs::create_symlink(dst_mnt, src_mnt / out_symlink, ec);
        REQUIRE(!ec);


        // create required output directories
        env.add_to_namespace(nsid1, dst_subdir1);

        // cp -r /a/contents.* -> / = /contents.*
        WHEN("copying the contents of a NORNS_REMOTE_PATH subdir from SRC "
             "namespace's root to a NORNS_LOCAL_PATH at DST namespace's root\n"
             "  cp -r /a/contents.* -> / = /contents.* ") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host,
                                               src_subdir0.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_root.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Copied subdirs are identical to original") {
                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_subdir0);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_root);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r /a/b/c/.../contents.* -> / = /contents.*
        WHEN("copying the contents of a NORNS_REMOTE_PATH arbitrary subdir to "
             "a NORNS_LOCAL_PATH at DST namespace's root\n"
             "  cp -r /a/b/c/.../contents.* -> / = /contents.*") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               src_subdir1.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_root.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Copied subdirs are identical to original") {
                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_subdir1);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_root);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r /a/contents.* -> /c = /c/contents.*
        // (c did not exist previously)
        WHEN("copying the contents of a NORNS_REMOTE_PATH subdir from SRC "
             "namespace's root to another NORNS_LOCAL_PATH subdir at DST "
             "namespace's root while changing its name\n"
             "  cp -r /a/contents.* -> /c = /c/contents.*") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               src_subdir0.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_subdir0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Copied subdirs are identical to original") {
                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_subdir0);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_subdir0);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r /a/contents.* -> /c = /c/contents.*
        // (c did exist previously)
        WHEN("copying the contents of a NORNS_REMOTE_PATH subdir from SRC "
             "namespace's root to another NORNS_LOCAL_PATH subdir at DST "
             "namespace's root while changing its name\n"
             "  cp -r /a/contents.* -> /c / /c/contents.*") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               src_subdir0.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_subdir1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Copied files are identical to original") {
                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_subdir0);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_subdir1);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r /a/b/c/.../contents.* -> /c = /c/a/b/c/.../contents.*
        // (c did not exist previously)
        WHEN("copying the contents of a NORNS_REMOTE_PATH subdir from SRC "
             "namespace's root to a NORNS_LOCAL_PATH subdir at DST "
             "namespace's root while changing its name\n"
             "cp -r /a/b/c/.../contents.* -> /c = /c/a/b/c/.../contents.*") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               src_subdir1.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_subdir0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Copied files are identical to original") {
                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_subdir1);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_subdir0);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        // cp -r /a/b/c/.../contents.* -> /c = /c/a/b/c/.../contents.*
        // (c did exist previously)
        WHEN("copying the contents of a NORNS_REMOTE_PATH subdir from SRC "
             "namespace's root to another NORNS_LOCAL_PATH subdir at DST "
             "namespace's root while changing its name:"
             "  cp -r /a/b/c/.../contents.* -> /c = /c/a/b/c/.../contents.*") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               src_subdir1.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_subdir1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Copied files are identical to original") {
                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_subdir1);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_subdir1);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        env.notify_success();
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("attempting to request a transfer") {

            norns_iotask_t task = NORNSCTL_IOTASK(
                NORNS_IOTASK_COPY,
                NORNS_LOCAL_PATH("nvml0://", "/a/b/c/"),
                NORNS_REMOTE_PATH("nvml0://", "node1", "/a/b/d/"));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ECONNFAILED") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
#endif
}

/******************************************************************************/
/* tests for pull transfers (links)                                           */
/******************************************************************************/
SCENARIO("copy remote POSIX path to local POSIX path involving links (admin)", 
         "[api::nornsctl_submit_pull_links]") {
    GIVEN("a running urd instance") {

        /**********************************************************************/
        /* setup common environment                                           */
        /**********************************************************************/
        test_env env(false);

        const char* nsid0 = "tmp0";
        const char* nsid1 = "tmp1";
        const char* remote_host = "127.0.0.1:42000";
        bfs::path src_mnt, dst_mnt;

        // create namespaces
        std::tie(std::ignore, src_mnt) = 
            env.create_namespace(nsid0, "mnt/tmp0", 16384);
        std::tie(std::ignore, dst_mnt) = 
            env.create_namespace(nsid1, "mnt/tmp1", 16384);

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

        // create input data
        env.add_to_namespace(nsid0, src_file_at_root, 40000);
        env.add_to_namespace(nsid0, src_file_at_subdir, 80000);
        env.add_to_namespace(nsid0, src_subdir0);
        env.add_to_namespace(nsid0, src_subdir1);
        env.add_to_namespace(nsid0, src_empty_dir);

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir0 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        for(int i=0; i<10; ++i) {
            const bfs::path p{src_subdir1 / ("file" + std::to_string(i))};
            env.add_to_namespace(nsid0, p, 4096+i*10);
        }

        // create input data with special permissions
        auto p = env.add_to_namespace(nsid0, src_noperms_file0, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file1, 0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_file2, 0);
        env.remove_access(p.parent_path());

        p = env.add_to_namespace(nsid0, src_noperms_subdir0);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir1);
        env.remove_access(p);

        p = env.add_to_namespace(nsid0, src_noperms_subdir2);
        env.remove_access(p.parent_path());

        // add symlinks to the namespace
        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_root0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_root1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_root2);

        env.add_to_namespace(nsid0, src_file_at_root, src_symlink_at_subdir0);
        env.add_to_namespace(nsid0, src_subdir0, src_symlink_at_subdir1);
        env.add_to_namespace(nsid0, src_subdir1, src_symlink_at_subdir2);

        // manually create a symlink leading outside namespace 0
        boost::system::error_code ec;
        const bfs::path out_symlink = "/out_symlink";
        bfs::create_symlink(dst_mnt, src_mnt / out_symlink, ec);
        REQUIRE(!ec);


        // create required output directories
        env.add_to_namespace(nsid1, dst_subdir1);

        /**********************************************************************/
        /* begin tests                                                        */
        /**********************************************************************/
        WHEN("copying a single NORNS_REMOTE_PATH file from SRC namespace's '/' "
             "through a symlink also located at '/'" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                              remote_host, 
                                              src_symlink_at_root0.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {

                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_symlink_at_root0);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_file_at_root0);

                            REQUIRE(compare_files(src, dst) == true);
                        }
                    }
                }
            }
        }

        WHEN("copying a single NORNS_REMOTE_PATH subdir from SRC "
             "namespace's '/' through a symlink also located at '/'" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               src_symlink_at_root1.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Directories are equal") {

                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_symlink_at_root1);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_file_at_root0);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        WHEN("copying a single NORNS_REMOTE_PATH arbitrary subdir"
             "through a symlink also located at '/'" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host,
                                               src_symlink_at_root2.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Directories are equal") {

                            bfs::path src = 
                                env.get_from_namespace(nsid0, src_symlink_at_root2);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_file_at_root0);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        WHEN("copying a single NORNS_REMOTE_PATH file from SRC namespace's '/' "
             "through a symlink located in a subdir" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               src_symlink_at_subdir0.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Files are equal") {

                            bfs::path src = 
                                env.get_from_namespace(nsid0, 
                                                    src_symlink_at_subdir0);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_file_at_root0);

                            REQUIRE(compare_files(src, dst) == true);
                        }
                    }
                }
            }
        }

        WHEN("copying a single NORNS_REMOTE_PATH subdir from SRC "
             "namespace's '/' through a symlink also located at subdir" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host, 
                                               src_symlink_at_subdir1.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Directories are equal") {

                            bfs::path src = 
                                env.get_from_namespace(nsid0, 
                                                    src_symlink_at_subdir1);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_file_at_root0);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        WHEN("copying a single NORNS_REMOTE_PATH arbitrary subdir"
             "through a symlink also located at a subdir" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY,
                             NORNS_REMOTE_PATH(nsid0, 
                                               remote_host,
                                               src_symlink_at_subdir2.c_str()),
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_EFINISHED") {
                        norns_stat_t stats;
                        rv = nornsctl_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);

                        THEN("Directories are equal") {

                            bfs::path src = 
                                env.get_from_namespace(nsid0, 
                                                    src_symlink_at_subdir2);
                            bfs::path dst = 
                                env.get_from_namespace(nsid1, dst_file_at_root0);

                            REQUIRE(compare_directories(src, dst) == true);
                        }
                    }
                }
            }
        }

        env.notify_success();
    }
}
