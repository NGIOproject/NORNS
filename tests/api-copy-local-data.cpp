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

#include "norns.h"
#include "test-env.hpp"
#include "compare-files.hpp"
#include "catch.hpp"

namespace bfs = boost::filesystem;

SCENARIO("copy local POSIX file to local POSIX file", 
         "[api::norns_submit_copy_local_posix_files]") {
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
        // - trying to copy a non-existing file
        WHEN("copying a non-existing NORNS_LOCAL_PATH file") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, src_invalid_file.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("norns_error() reports NORNS_ESYSTEMERROR and "
                         "ENOENT") {
                        norns_stat_t stats;
                        rv = norns_error(&task, &stats);

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
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, src_invalid_dir.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_wait() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("norns_error() reports NORNS_ESYSTEMERROR and "
                         "ENOENT") {
                        norns_stat_t stats;
                        rv = norns_error(&task, &stats);

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
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, src_empty_dir.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("norns_error() reports NORNS_SUCCESS and ENOENT") {
                        norns_stat_t stats;
                        rv = norns_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHED);
                        REQUIRE(stats.st_task_error == NORNS_SUCCESS);
                        REQUIRE(stats.st_sys_errno == 0);

                        REQUIRE(bfs::exists(dst_mnt / dst_file_at_root0));
                    }
                }
            }
        }

//FIXME: DISABLED in CI until impersonation is implemented or 
// capabilities can be added to the docker service
#ifdef __SETCAP_TESTS__

        // - trying to copy a file from namespace root with invalid access 
        // permissions
        WHEN("copying a NORNS_LOCAL_PATH file from \"/\" without appropriate "
             "permissions to access it") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, src_noperms_file0.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("norns_error() reports NORNS_ESYSTEMERROR and "
                         "EACCES|EPERM|EINVAL") {
                        norns_stat_t stats;
                        rv = norns_error(&task, &stats);

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

        // - trying to copy a file from namespace root with invalid access
        // permissions
        WHEN("copying a NORNS_LOCAL_PATH file from a subdir without "
             "appropriate permissions to access it") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, src_noperms_file1.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("norns_error() reports NORNS_ESYSTEMERROR and "
                         "EACCES|EPERM|EINVAL") {
                        norns_stat_t stats;
                        rv = norns_error(&task, &stats);

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

        // - trying to copy a file from namespace root with invalid access
        // permissions
        WHEN("copying a NORNS_LOCAL_PATH file from a subdir without "
             "appropriate permissions to access a parent") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, src_noperms_file2.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("norns_error() reports NORNS_ESYSTEMERROR and "
                         "EACCES|EPERM|EINVAL") {
                        norns_stat_t stats;
                        rv = norns_error(&task, &stats);

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

        // - trying to copy a subdir from namespace root with invalid access
        // permissions
        WHEN("copying a NORNS_LOCAL_PATH subdir from \"/\"  without "
             "appropriate permissions to access it") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_noperms_subdir0.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("norns_error() reports NORNS_ESYSTEMERROR and "
                         "EACCES|EPERM|EINVAL") {
                        norns_stat_t stats;
                        rv = norns_error(&task, &stats);

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

        // - trying to copy a subdir from namespace root with invalid access
        // permissions
        WHEN("copying a NORNS_LOCAL_PATH subdir from another subdir without "
             "appropriate permissions to access it") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_noperms_subdir1.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("norns_error() reports NORNS_ESYSTEMERROR and "
                         "EACCES|EPERM|EINVAL") {
                        norns_stat_t stats;
                        rv = norns_error(&task, &stats);

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

        // - trying to copy a subdir from namespace root with invalid 
        // access permissions
        WHEN("copying a NORNS_LOCAL_PATH subdir from another subdir without "
             "appropriate permissions to access a parent") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_noperms_subdir2.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("norns_error() reports NORNS_ESYSTEMERROR and "
                         "EACCES|EPERM|EINVAL") {
                        norns_stat_t stats;
                        rv = norns_error(&task, &stats);

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

        // symlink leading out of namespace
        WHEN("copying a NORNS_LOCAL_PATH through a symbolic link that leads "
             "out of the src namespace") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, out_symlink.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("norns_error() reports NORNS_ESYSTEMERROR and "
                         "ENOENT") {
                        norns_stat_t stats;
                        rv = norns_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(stats.st_sys_errno == ENOENT);
                    }
                }
            }
        }


        /**********************************************************************/
        /* tests for single files                                             */
        /**********************************************************************/
        // cp -r ns0://file0.txt -> ns1:// = ns1://file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from src namespace's root to "
             "another NORNS_LOCAL_PATH at dst namespace's root (keeping the "
             "name)") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, src_file_at_root.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

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

        // cp -r ns0://file0.txt -> ns1://file1.txt = ns1://file1.txt
        WHEN("copying a single NORNS_LOCAL_PATH from src namespace's root to "
             "another NORNS_LOCAL_PATH at dst namespace's root (changing the "
             "name)") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, src_file_at_root.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root1.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

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

        // cp -r ns0://a/b/c/.../d/file0.txt -> 
        //       ns1://file0.txt = ns1://file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a src namespace's subdir "
             "to another NORNS_LOCAL_PATH at dst namespace's root "
             "(keeping the name)") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_file_at_subdir.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("Files are equal") {
                        bfs::path src = 
                            env.get_from_namespace(nsid0, src_file_at_subdir);
                        bfs::path dst = 
                            env.get_from_namespace(nsid1, dst_file_at_root0);

                        REQUIRE(compare_files(src, dst) == true);
                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../d/file0.txt -> 
        //       ns1://file1.txt = ns1://file1.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a src namespace's subdir "
             "to another NORNS_LOCAL_PATH at dst namespace's root (changing "
             "the name)") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_file_at_subdir.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root1.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("Files are equal") {
                        bfs::path src = 
                            env.get_from_namespace(nsid0, src_file_at_subdir);
                        bfs::path dst = 
                            env.get_from_namespace(nsid1, dst_file_at_root1);

                        REQUIRE(compare_files(src, dst) == true);
                    }
                }
            }
        }

        // cp -r ns0://file0.txt -> 
        //       ns1://a/b/c/.../file0.txt = ns1://a/b/c/.../file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from src namespace's root to "
             "another NORNS_LOCAL_PATH at a dst namespace's subdir (keeping "
             "the name)") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_file_at_root.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_subdir0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("Files are equal") {
                        bfs::path src = 
                            env.get_from_namespace(nsid0, src_file_at_root);
                        bfs::path dst = 
                            env.get_from_namespace(nsid1, dst_file_at_subdir0);

                        REQUIRE(compare_files(src, dst) == true);
                    }
                }
            }
        }

        // cp -r ns0://file0.txt -> 
        //       ns1://a/b/c/.../file1.txt = ns1://a/b/c/.../file1.txt
        WHEN("copying a single NORNS_LOCAL_PATH from src namespace's root to "
             "another NORNS_LOCAL_PATH at a dst namespace's subdir (changing "
             "the name)") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_file_at_root.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_subdir1.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("Files are equal") {
                        bfs::path src = 
                            env.get_from_namespace(nsid0, src_file_at_root);
                        bfs::path dst = 
                            env.get_from_namespace(nsid1, dst_file_at_subdir1);

                        REQUIRE(compare_files(src, dst) == true);
                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../file0.txt -> 
        //       ns1://a/b/c/.../file0.txt = ns1://a/b/c/.../file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a src namespace's subdir "
             "to another NORNS_LOCAL_PATH at a dst namespace's subdir (keeping "
             "the name)") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_file_at_subdir.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_subdir0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("Files are equal") {
                        bfs::path src = 
                            env.get_from_namespace(nsid0, src_file_at_subdir);
                        bfs::path dst = 
                            env.get_from_namespace(nsid1, dst_file_at_subdir0);

                        REQUIRE(compare_files(src, dst) == true);
                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../file0.txt -> 
        //       ns1://a/b/c/.../file1.txt = ns1://a/b/c/.../file1.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a src namespace's subdir "
             "to another NORNS_LOCAL_PATH at a dst namespace's subdir "
             "(changing the name)") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_file_at_subdir.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_subdir1.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("Files are equal") {
                        bfs::path src = 
                            env.get_from_namespace(nsid0, src_file_at_subdir);
                        bfs::path dst = 
                            env.get_from_namespace(nsid1, dst_file_at_subdir1);

                        REQUIRE(compare_files(src, dst) == true);
                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../file0.txt -> 
        //       ns1://e/f/g/.../file0.txt = ns1://e/f/g/.../file0.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a src namespace's subdir "
             "to another NORNS_LOCAL_PATH at a dst namespace's subdir "
             "(changing the parents names)") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_file_at_subdir.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_subdir2.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("Files are equal") {
                        bfs::path src = 
                            env.get_from_namespace(nsid0, src_file_at_subdir);
                        bfs::path dst = 
                            env.get_from_namespace(nsid1, dst_file_at_subdir2);

                        REQUIRE(compare_files(src, dst) == true);
                    }
                }
            }
        }

        // cp -r ns0://a/b/c/.../file0.txt -> 
        //       ns1://e/f/g/.../file1.txt = ns1://e/f/g/.../file1.txt
        WHEN("copying a single NORNS_LOCAL_PATH from a src namespace's subdir "
            "to another NORNS_LOCAL_PATH at a dst namespace's subdir (changing "
            "the name)") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_file_at_subdir.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_subdir3.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("Files are equal") {
                        bfs::path src = 
                            env.get_from_namespace(nsid0, src_file_at_subdir);
                        bfs::path dst = 
                            env.get_from_namespace(nsid1, dst_file_at_subdir3);

                        REQUIRE(compare_files(src, dst) == true);
                    }
                }
            }
        }

        /**********************************************************************/
        /* tests for directories                                              */
        /**********************************************************************/
        // cp -r /a/contents.* -> / = /contents.*
        WHEN("copying the contents of a NORNS_LOCAL_PATH subdir from src "
             "namespace's root to dst namespace's root") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, src_subdir0.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, dst_root.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

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

        // cp -r /a/b/c/.../contents.* -> / = /contents.*
        WHEN("copying the contents of a NORNS_LOCAL_PATH arbitrary subdir to "
             "dst namespace's root") {
            
            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, src_subdir1.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, dst_root.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

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

        // cp -r /a/contents.* -> /c = /c/contents.*
        // (c did not exist previously)
        WHEN("copying the contents of a NORNS_LOCAL_PATH subdir from src "
             "namespace's root to another NORNS_LOCAL_PATH subdir at dst "
             "namespace's root while changing its name") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, src_subdir0.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, dst_subdir0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

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

        // cp -r /a/contents.* -> /c = /c/contents.*
        // (c did exist previously)
        WHEN("copying the contents of a NORNS_LOCAL_PATH subdir from src "
             "namespace's root to another NORNS_LOCAL_PATH subdir at dst "
             "namespace's root while changing its name") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, src_subdir0.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, dst_subdir1.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

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

        // cp -r /a/b/c/.../contents.* -> /c = /c/a/b/c/.../contents.*
        // (c did not exist previously)
        WHEN("copying the contents of a NORNS_LOCAL_PATH subdir from src "
             "namespace's root to another NORNS_LOCAL_PATH subdir at dst "
             "namespace's root while changing its name") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, src_subdir1.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, dst_subdir0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

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

        // cp -r /a/b/c/.../contents.* -> /c = /c/a/b/c/.../contents.*
        // (c did exist previously)
        WHEN("copying the contents of a NORNS_LOCAL_PATH subdir from src "
             "namespace's root to another NORNS_LOCAL_PATH subdir at dst "
             "namespace's root while changing its name") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, src_subdir1.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, dst_subdir1.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

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

        /**********************************************************************/
        /* tests for soft links                                               */
        /**********************************************************************/
        WHEN("copying a single NORNS_LOCAL_PATH file from src namespace's '/' "
             "through a symlink also located at '/'" ) {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_symlink_at_root0.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

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

        WHEN("copying a single NORNS_LOCAL_PATH subdir from src namespace's "
             "'/' through a symlink also located at '/'" ) {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_symlink_at_root1.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

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

        WHEN("copying a single NORNS_LOCAL_PATH arbitrary subdir"
             "through a symlink also located at '/'" ) {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_symlink_at_root2.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

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

        WHEN("copying a single NORNS_LOCAL_PATH file from src namespace's '/' "
             "through a symlink located in a subdir" ) {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_symlink_at_subdir0.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

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

        WHEN("copying a single NORNS_LOCAL_PATH subdir from src namespace's "
             "'/' through a symlink also located at subdir" ) {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_symlink_at_subdir1.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("Directories are equal") {

                        bfs::path src = 
                            env.get_from_namespace(nsid0, src_symlink_at_subdir1);
                        bfs::path dst = 
                            env.get_from_namespace(nsid1, dst_file_at_root0);

                        REQUIRE(compare_directories(src, dst) == true);
                    }
                }
            }
        }

        WHEN("copying a single NORNS_LOCAL_PATH arbitrary subdir"
             "through a symlink also located at a subdir" ) {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_LOCAL_PATH(nsid0, 
                                              src_symlink_at_subdir2.c_str()), 
                             NORNS_LOCAL_PATH(nsid1, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() returns NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

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

        env.notify_success();
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


SCENARIO("copy local memory buffer to local POSIX file", 
         "[api::norns_submit_copy_buffer_to_file]") {
    GIVEN("a running urd instance") {

        test_env env;

        const char* nsid0 = "tmp0";
        bfs::path dst_mnt;

        // create namespaces
        std::tie(std::ignore, dst_mnt) = 
            env.create_namespace(nsid0, "mnt/tmp0", 16384);

        // create input data buffer
        std::vector<int> input_data(100, 42);
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

        /**************************************************************************************************************/
        /* tests for error conditions                                                                                 */
        /**************************************************************************************************************/
        //TODO
        // - copy a valid but removed memory region 
        //   (cannot control => undefined behavior)
        // - providing a non-existing directory path (i.e. finished with /) as 
        //   output name
        // - providing an existing path that points to a directory as 
        //   output name
        WHEN("copying an invalid memory region to a local POSIX file") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_MEMORY_REGION((void*) 0x42, region_size), 
                             NORNS_LOCAL_PATH(nsid0, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() return NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("norns_error() reports NORNS_ESYSTEMERROR and "
                         "EFAULT") {
                        norns_stat_t stats;
                        rv = norns_error(&task, &stats);

                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(stats.st_sys_errno == EFAULT);
                    }
                }
            }
        }

        WHEN("copying a valid memory region to a local POSIX file located "
             "at '/'") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_MEMORY_REGION(region_addr, region_size), 
                             NORNS_LOCAL_PATH(nsid0, 
                                              dst_file_at_root0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() return NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("Output file contains buffer data") {

                        bfs::path dst = 
                            env.get_from_namespace(nsid0, dst_file_at_root0);

                        REQUIRE(compare(input_data, dst) == true);
                    }
                }
            }
        }

        WHEN("copying a valid memory region to a local POSIX file located at a "
             "subdir") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_MEMORY_REGION(region_addr, region_size), 
                             NORNS_LOCAL_PATH(nsid0, 
                                              dst_file_at_subdir0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() return NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("Output file contains buffer data") {

                        bfs::path dst = 
                            env.get_from_namespace(nsid0, dst_file_at_subdir0);

                        REQUIRE(compare(input_data, dst) == true);
                    }
                }
            }
        }

        WHEN("copying a valid memory region to a local POSIX /") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_MEMORY_REGION(region_addr, region_size), 
                             NORNS_LOCAL_PATH(nsid0, dst_root.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_EBADARGS") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("copying a valid memory region to a local POSIX existing "
             "directory at /") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_MEMORY_REGION(region_addr, region_size), 
                             NORNS_LOCAL_PATH(nsid0, dst_subdir0.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_EBADARGS") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("copying a valid memory region to a local POSIX existing "
             "directory at /") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_MEMORY_REGION(region_addr, region_size), 
                             NORNS_LOCAL_PATH(nsid0, dst_subdir1.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() return NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("norns_error() reports NORNS_ESYSTEMERROR and "
                         "EISDIR") {
                        norns_stat_t stats;
                        rv = norns_error(&task, &stats);

                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(stats.st_sys_errno == EISDIR);
                    }
                }
            }
        }

        WHEN("copying a valid memory region to a local POSIX existing "
             "directory at an arbitary subdir") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_MEMORY_REGION(region_addr, region_size), 
                             NORNS_LOCAL_PATH(nsid0, dst_subdir2.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_EBADARGS") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("copying a valid memory region to a local POSIX existing "
             "directory at an arbitary subdir") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_MEMORY_REGION(region_addr, region_size), 
                             NORNS_LOCAL_PATH(nsid0, dst_subdir3.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                // wait until the task completes
                rv = norns_wait(&task, NULL);

                THEN("norns_wait() return NORNS_SUCCESS") {
                    REQUIRE(rv == NORNS_SUCCESS);

                    THEN("norns_error() reports NORNS_ESYSTEMERROR and "
                         "EISDIR") {
                        norns_stat_t stats;
                        rv = norns_error(&task, &stats);

                        REQUIRE(stats.st_status == NORNS_EFINISHEDWERROR);
                        REQUIRE(stats.st_task_error == NORNS_ESYSTEMERROR);
                        REQUIRE(stats.st_sys_errno == EISDIR);
                    }
                }
            }
        }


        // i.e. a destination path that 'looks like' a directory
        WHEN("copying a valid memory region to a local POSIX non-existing "
             "directory") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_MEMORY_REGION(region_addr, region_size), 
                             NORNS_LOCAL_PATH(nsid0, dst_subdir4.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_EBADARGS") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("copying a valid memory region to a local POSIX non-existing "
             "directory") {

            norns_iotask_t task = 
                NORNS_IOTASK(NORNS_IOTASK_COPY, 
                             NORNS_MEMORY_REGION(region_addr, region_size), 
                             NORNS_LOCAL_PATH(nsid0, dst_subdir5.c_str()));

            norns_error_t rv = norns_submit(&task);

            THEN("norns_submit() returns NORNS_EBADARGS") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        env.notify_success();
    }
}
