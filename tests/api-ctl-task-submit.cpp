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
#include "catch.hpp"

SCENARIO("submit control request", "[api::nornsctl_submit]") {
    GIVEN("a running urd instance") {

        test_env env(
            fake_daemon_cfg {
                true /* dry_run? */
            }
        );

        const char* nsid0 = "tmp0";
        const char* nsid1 = "tmp1";
        const char* nsid3 = "lustre0";
        bfs::path tmp0_mnt, tmp1_mnt, lustre_mnt;

        // create namespaces
        std::tie(std::ignore, tmp0_mnt) =
            env.create_namespace(nsid0, "mnt/tmp0", 16384);
        std::tie(std::ignore, tmp1_mnt) =
            env.create_namespace(nsid1, "mnt/tmp1", 16384);
        std::tie(std::ignore, lustre_mnt) =
            env.create_namespace(nsid3, "mnt/lustre0", 16384);

        // define input names
        const char* src_host0 = "node0";
        const bfs::path src_file0 = "/a/b/c";
        const bfs::path src_file1 = "/b/c/d";
        void* src_mem_addr = (void*) 0xdeadbeef;
        const size_t src_mem_size = (size_t) 42;

        // define output names
        const char* dst_host0 = "node0";
        const bfs::path dst_file0 = "/a/b/c";
        const bfs::path dst_file1 = "/b/c/d";
        void* dst_mem_addr = (void*) 0xdeadbeef;
        const size_t dst_mem_size = (size_t) 42;

        /**********************************************************************/
        /* tests for error conditions                                         */
        /**********************************************************************/
        WHEN("submitting a request to copy data using unregistered "
             "namespaces") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_MEMORY_REGION(src_mem_addr, src_mem_size), 
                                NORNS_LOCAL_PATH("tmp2", dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ENOSUCHNAMESPACE") {
                REQUIRE(rv == NORNS_ENOSUCHNAMESPACE);
            }
        }

        WHEN("submitting a request to copy data using an unregistered src "
             "namespace") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_LOCAL_PATH("tmp2", src_file0.c_str()), 
                                NORNS_SHARED_PATH(nsid3, dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ENOSUCHNAMESPACE") {
                REQUIRE(rv == NORNS_ENOSUCHNAMESPACE);
            }
        }

        WHEN("submitting a request to copy data using an unregistered dst "
             "namespace") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_LOCAL_PATH(nsid0, src_file0.c_str()), 
                                NORNS_SHARED_PATH("tmp2", dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ENOSUCHNAMESPACE") {
                REQUIRE(rv == NORNS_ENOSUCHNAMESPACE);
            }
        }


        /**********************************************************************/
        /* tests for NORNS_IOTASK_COPY                                        */
        /**********************************************************************/
        /* copy from process memory to .* */
        WHEN("submitting a request to copy from NORNS_MEMORY_REGION to "
             "NORNS_LOCAL_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_MEMORY_REGION(src_mem_addr, src_mem_size), 
                                NORNS_LOCAL_PATH(nsid0, dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }
        }

        WHEN("submitting a request to copy from NORNS_MEMORY_REGION to "
             "NORNS_SHARED_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_MEMORY_REGION(src_mem_addr, src_mem_size), 
                                NORNS_SHARED_PATH(nsid0, dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }
        }

        WHEN("submitting a request to copy from a NORNS_MEMORY_REGION to "
             "a NORNS_REMOTE_PATH") {
            
            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_MEMORY_REGION(src_mem_addr, src_mem_size), 
                                NORNS_REMOTE_PATH(nsid0, 
                                                  dst_host0, 
                                                  dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        /* copy from local path to .* */
        WHEN("submitting a request to copy from NORNS_LOCAL_PATH to "
             "NORNS_LOCAL_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_LOCAL_PATH(nsid0, src_file1.c_str()), 
                                NORNS_LOCAL_PATH(nsid1, dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }
        }

        WHEN("submitting a request to copy from NORNS_LOCAL_PATH to "
             "NORNS_SHARED_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_LOCAL_PATH(nsid0, src_file1.c_str()), 
                                NORNS_SHARED_PATH(nsid3, dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }
        }

        WHEN("submitting a request to copy from NORNS_LOCAL_PATH to "
             "NORNS_REMOTE_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_LOCAL_PATH(nsid0, src_file1.c_str()), 
                                NORNS_REMOTE_PATH(nsid1, 
                                                  dst_host0, 
                                                  dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_REMOTE_PATH to "
             "NORNS_LOCAL_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_REMOTE_PATH(nsid0, 
                                                  src_host0, 
                                                  src_file0.c_str()),
                                NORNS_LOCAL_PATH(nsid1, 
                                                 dst_file1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() return NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_REMOTE_PATH to "
             "NORNS_SHARED_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_REMOTE_PATH(nsid0, 
                                                  src_host0, 
                                                  src_file0.c_str()),
                                NORNS_SHARED_PATH(nsid1, dst_file1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ENOTSUPPORTED") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_REMOTE_PATH to "
             "NORNS_REMOTE_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_REMOTE_PATH(nsid0, 
                                                  src_host0, 
                                                  src_file0.c_str()),
                                NORNS_REMOTE_PATH(nsid1, 
                                                  dst_host0, 
                                                  dst_file1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ENOTSUPPORTED") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_LOCAL_PATH to "
             "NORNS_MEMORY_REGION") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_LOCAL_PATH(nsid0, src_file0.c_str()), 
                                NORNS_MEMORY_REGION(dst_mem_addr, 
                                                    dst_mem_size));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ENOTSUPPORTED") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_SHARED_PATH to "
             "NORNS_MEMORY_REGION") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_SHARED_PATH(nsid3, src_file0.c_str()), 
                                NORNS_MEMORY_REGION(dst_mem_addr, 
                                                    dst_mem_size));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ENOTSUPPORTED") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_REMOTE_PATH to "
             "NORNS_MEMORY_REGION") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_REMOTE_PATH(nsid0, 
                                                  src_host0, 
                                                  src_file0.c_str()), 
                                NORNS_MEMORY_REGION(dst_mem_addr, 
                                                    dst_mem_size));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ENOTSUPPORTED") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }
        }


        /**********************************************************************/
        /* tests for NORNS_IOTASK_MOVE                                     */
        /**********************************************************************/
        /* move from process memory to .* */
        WHEN("submitting a request to move from NORNS_MEMORY_REGION to "
             "NORNS_LOCAL_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                NORNS_MEMORY_REGION(src_mem_addr, src_mem_size), 
                                NORNS_LOCAL_PATH(nsid1, dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }
        }

        WHEN("submitting a request to move from NORNS_MEMORY_REGION to "
             "NORNS_SHARED_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                NORNS_MEMORY_REGION(src_mem_addr, src_mem_size), 
                                NORNS_SHARED_PATH(nsid1, dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }
        }

        WHEN("submitting a request to move from NORNS_MEMORY_REGION to "
             "NORNS_REMOTE_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                NORNS_MEMORY_REGION(src_mem_addr, src_mem_size), 
                                NORNS_REMOTE_PATH(nsid1, 
                                                  dst_host0, 
                                                  dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        /* move from local path to .* */
        WHEN("submitting a request to move from NORNS_LOCAL_PATH to "
             "NORNS_LOCAL_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                NORNS_LOCAL_PATH(nsid0, src_file1.c_str()), 
                                NORNS_LOCAL_PATH(nsid1, dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }
        }

        WHEN("submitting a request to move from NORNS_LOCAL_PATH to "
             "NORNS_SHARED_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                NORNS_LOCAL_PATH(nsid0, src_file1.c_str()), 
                                NORNS_SHARED_PATH(nsid3, dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }
        }

        WHEN("submitting a request to move from NORNS_LOCAL_PATH to "
             "NORNS_REMOTE_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                NORNS_LOCAL_PATH(nsid0, src_file1.c_str()), 
                                NORNS_REMOTE_PATH(nsid1, 
                                                  dst_host0, 
                                                  dst_file0.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_REMOTE_PATH to "
             "NORNS_LOCAL_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                NORNS_REMOTE_PATH(nsid0, 
                                                  src_host0, 
                                                  src_file0.c_str()),
                                NORNS_LOCAL_PATH(nsid1, 
                                                 dst_file1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_REMOTE_PATH to "
             "NORNS_SHARED_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                NORNS_REMOTE_PATH(nsid0, 
                                                  src_host0, 
                                                  src_file0.c_str()),
                                NORNS_SHARED_PATH(nsid1, dst_file1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ENOTSUPPORTED") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_REMOTE_PATH to "
             "NORNS_REMOTE_PATH") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                NORNS_REMOTE_PATH(nsid0, 
                                                  src_host0, 
                                                  src_file0.c_str()),
                                NORNS_REMOTE_PATH(nsid1, 
                                                  dst_host0, 
                                                  dst_file1.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ENOTSUPPORTED") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_LOCAL_PATH to "
             "NORNS_MEMORY_REGION") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                NORNS_LOCAL_PATH(nsid0, src_file0.c_str()), 
                                NORNS_MEMORY_REGION(dst_mem_addr, 
                                                    dst_mem_size));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ENOTSUPPORTED") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_SHARED_PATH to "
             "NORNS_MEMORY_REGION") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                             NORNS_SHARED_PATH(nsid3, src_file0.c_str()), 
                             NORNS_MEMORY_REGION(dst_mem_addr, dst_mem_size));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ENOTSUPPORTED") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_REMOTE_PATH to "
             "NORNS_MEMORY_REGION") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                NORNS_REMOTE_PATH(nsid0, 
                                                  src_host0, 
                                                  src_file0.c_str()), 
                                NORNS_MEMORY_REGION(dst_mem_addr, 
                                                    dst_mem_size));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ENOTSUPPORTED") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }
        }

        env.notify_success();
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("attempting to request a transfer") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_LOCAL_PATH("nvml0://", "/a/b/c/"),
                                NORNS_REMOTE_PATH("nvml0://", 
                                                  "node1", 
                                                  "/a/b/d/"));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_ECONNFAILED") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
#endif
}
