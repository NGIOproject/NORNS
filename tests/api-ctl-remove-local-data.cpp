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

SCENARIO("remove a local POSIX file (admin)", 
         "[api::nornsctl_submit_remove_local_posix_files]") {
    GIVEN("a running urd instance") {

        test_env env;

        const char* nsid0 = "tmp0";
        const char* nsid1 = "tmp1";
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
        env.add_to_namespace(nsid0, src_file_at_root, 4096);
        env.add_to_namespace(nsid0, src_file_at_subdir, 8192);
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
        /* tests for error conditions                                         */
        /**********************************************************************/
        // - trying to remove a non-existing file
        WHEN("removing a non-existing NORNS_LOCAL_PATH file") {
            
            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_REMOVE, 
                             NORNS_LOCAL_PATH(nsid0, src_invalid_file.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_ESYSTEMERROR and "
                         "ENOENT") {
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


        // - trying to remove a non-existing directory
        WHEN("removing a non-existing NORNS_LOCAL_PATH directory") {
            
            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_REMOVE, 
                             NORNS_LOCAL_PATH(nsid0, src_invalid_dir.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("nornsctl_error() reports NORNS_ESYSTEMERROR and "
                         "ENOENT") {
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

//FIXME: DISABLED in CI until impersonation is implemented or capabilities can be added to the docker service
#ifdef __SETCAP_TESTS__

        // - trying to copy a file from namespace root with invalid access permissions
        WHEN("copying a NORNS_LOCAL_PATH file from \"/\" without appropriate "
             "permissions to access it") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                 src_noperms_file0.c_str()), 
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

                    THEN("nornsctl_error() reports NORNS_ESYSTEMERROR and "
                         "EACCES|EPERM|EINVAL") {
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

                    THEN("nornsctl_error() reports NORNS_ESYSTEMERROR and "
                         "EACCES|EPERM|EINVAL") {
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

                    THEN("nornsctl_error() reports NORNS_ESYSTEMERROR and "
                         "EACCES|EPERM|EINVAL") {
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

                    THEN("nornsctl_error() reports NORNS_ESYSTEMERROR and "
                         "EACCES|EPERM|EINVAL") {
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
             "appropriate permissions to access it") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                 src_noperms_subdir1.c_str()), 
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

                    THEN("nornsctl_error() reports NORNS_ESYSTEMERROR and "
                         "EACCES|EPERM|EINVAL") {
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

                    THEN("nornsctl_error() reports NORNS_ESYSTEMERROR and "
                         "EACCES|EPERM|EINVAL") {
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
#endif

        /**********************************************************************/
        /* tests for single files                                             */
        /**********************************************************************/
        // rm ns0://file0.txt
        WHEN("removing a single NORNS_LOCAL_PATH from src namespace's root") {
            
            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_REMOVE, 
                             NORNS_LOCAL_PATH(nsid0, src_file_at_root.c_str()));
            const bfs::path p = env.get_from_namespace(nsid0, src_file_at_root);

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("File no longer exists") {
                        REQUIRE(!bfs::exists(p));
                    }
                }
            }
        }

        // rm ns0://a/b/c/.../d/file0.txt
        WHEN("removing a single NORNS_LOCAL_PATH from a src namespace's "
              "subdir") {
            
            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_REMOVE, 
                             NORNS_LOCAL_PATH(nsid0, 
                                 src_file_at_subdir.c_str()));
            const bfs::path p = 
                env.get_from_namespace(nsid0, src_file_at_subdir);

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("File no longer exists") {
                        REQUIRE(!bfs::exists(p));
                    }
                }
            }
        }

        /**********************************************************************/
        /* tests for directories                                              */
        /**********************************************************************/
        // rm -r /a/contents.*
        WHEN("removing the contents of a NORNS_LOCAL_PATH subdir from src "
             "namespace's root") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_REMOVE, 
                             NORNS_LOCAL_PATH(nsid0, src_subdir0.c_str()));
            const bfs::path p = env.get_from_namespace(nsid0, src_subdir0);

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("Directory no longer exists") {
                        REQUIRE(!bfs::exists(p));
                    }
                }
            }
        }

        // rm -r /a/b/c/.../contents.*
        WHEN("removing the contents of a NORNS_LOCAL_PATH arbitrary subdir") {
            
            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_REMOVE, 
                             NORNS_LOCAL_PATH(nsid0, src_subdir1.c_str()));
            const bfs::path p = env.get_from_namespace(nsid0, src_subdir1);

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("Directory no longer exists") {
                        REQUIRE(!bfs::exists(p));
                    }
                }
            }
        }

        WHEN("removing an empty NORNS_LOCAL_PATH directory") {
            
            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_REMOVE, 
                             NORNS_LOCAL_PATH(nsid0, src_empty_dir.c_str()));
            const bfs::path p = env.get_from_namespace(nsid0, src_empty_dir);

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("Directory no longer exists") {
                        REQUIRE(!bfs::exists(p));
                    }
                }
            }
        }


        /**********************************************************************/
        /* tests for soft links                                               */
        /**********************************************************************/
        WHEN("removing a single NORNS_LOCAL_PATH file from src namespace's '/' "
             "through a symlink also located at '/'" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_REMOVE, 
                             NORNS_LOCAL_PATH(nsid0, 
                                 src_symlink_at_root0.c_str()));
            const bfs::path p = 
                env.get_from_namespace(nsid0, "/") / src_symlink_at_root0;

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("symlink no longer exists and original file is left "
                         "intact") {
                        REQUIRE(!bfs::exists(p));
                        REQUIRE(bfs::exists(
                            env.get_from_namespace(nsid0, src_file_at_root)));
                    }
                }
            }
        }

        WHEN("removing a single NORNS_LOCAL_PATH arbitrary subdir"
             "through a symlink located at '/'" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_REMOVE, 
                             NORNS_LOCAL_PATH(nsid0, 
                                 src_symlink_at_root2.c_str()));
            const bfs::path p = 
                env.get_from_namespace(nsid0, "/") / src_symlink_at_root2;

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("symlink no longer exists and original file is left "
                         "intact") {
                        REQUIRE(!bfs::exists(p));
                        REQUIRE(bfs::exists(
                            env.get_from_namespace(nsid0, src_subdir1)));
                    }
                }
            }
        }

        WHEN("removing a single NORNS_LOCAL_PATH file from src namespace's '/' "
             "through a symlink located in a subdir" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_REMOVE, 
                             NORNS_LOCAL_PATH(nsid0, 
                                 src_symlink_at_subdir0.c_str()));
            const bfs::path p = 
                env.get_from_namespace(nsid0, "/") / src_symlink_at_subdir0;

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("symlink no longer exists and original file is left "
                         "intact") {
                        REQUIRE(!bfs::exists(p));
                        REQUIRE(bfs::exists(
                            env.get_from_namespace(nsid0, src_file_at_root)));
                    }
                }
            }
        }

        WHEN("removing a single NORNS_LOCAL_PATH subdir from src namespace's "
             "'/' through a symlink also located at subdir") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_REMOVE, 
                             NORNS_LOCAL_PATH(nsid0, 
                                 src_symlink_at_subdir1.c_str()));
            const bfs::path p = 
                env.get_from_namespace(nsid0, "/") / src_symlink_at_subdir1;

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("symlink no longer exists and original file is left "
                         "intact") {
                        REQUIRE(!bfs::exists(p));
                        REQUIRE(bfs::exists(env.get_from_namespace(nsid0, src_subdir0)));
                    }
                }
            }
        }

        WHEN("removing a single NORNS_LOCAL_PATH arbitrary subdir"
             "through a symlink also located at a subdir" ) {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_REMOVE, 
                             NORNS_LOCAL_PATH(nsid0, 
                                 src_symlink_at_subdir2.c_str()));
            const bfs::path p = 
                env.get_from_namespace(nsid0, "/") / src_symlink_at_subdir2;

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = nornsctl_wait(&task, NULL);

                THEN("nornsctl_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("symlink no longer exists and original file is left "
                         "intact") {
                        REQUIRE(!bfs::exists(p));
                        REQUIRE(bfs::exists(
                            env.get_from_namespace(nsid0, src_subdir1)));
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

