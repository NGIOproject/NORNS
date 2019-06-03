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

#include <string.h>
#include "nornsctl.h"
#include "catch.hpp"

SCENARIO("initialize a task with nornsctl_iotask_init", 
         "[api::nornsctl_iotask_init]") {

    GIVEN("invalid task information") {
        WHEN("initializing a task with NULL src and dst") {

            norns_iotask_t task;
            norns_op_t task_op = NORNS_IOTASK_COPY;

            nornsctl_iotask_init(&task, task_op, NULL, NULL);

            AND_THEN("task is set to 0") {
                norns_iotask_t dummy;
                memset(&dummy, 0, sizeof(dummy));
                REQUIRE(memcmp(&task, &dummy, sizeof(norns_iotask_t)) == 0);
            }
        }
    }

    GIVEN("invalid task information") {
        WHEN("initializing a task with a NULL src") {

            norns_iotask_t task;
            norns_op_t task_op = NORNS_IOTASK_COPY;

            const char* dst_nsid = "tmp://";
            const char* dst_path = "/a/b/c";
            norns_resource_t dst = NORNS_LOCAL_PATH(dst_nsid, dst_path);

            nornsctl_iotask_init(&task, task_op, NULL, &dst);

            THEN("task is set to 0") {
                norns_iotask_t dummy;
                memset(&dummy, 0, sizeof(dummy));
                REQUIRE(memcmp(&task, &dummy, sizeof(norns_iotask_t)) == 0);
            }
        }
    }

    GIVEN("valid task information") {
        WHEN("initializing a task with src=NORNS_PROCESS_MEMORY and "
            "dst=NORNS_POSIX_PATH | R_LOCAL") {

            norns_op_t task_op = NORNS_IOTASK_COPY;
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            const char* dst_nsid = "tmp://";
            const char* dst_path = "/a/b/c";

            norns_resource_t src = NORNS_MEMORY_REGION(src_addr, src_size);

            REQUIRE(src.r_buffer.b_addr == src_addr);
            REQUIRE(src.r_buffer.b_size == src_size);

            norns_resource_t dst = NORNS_LOCAL_PATH(dst_nsid, dst_path);

            REQUIRE(strcmp(dst.r_posix_path.p_nsid, dst_nsid) == 0);
            REQUIRE(dst.r_posix_path.p_host == NULL);
            REQUIRE(strcmp(dst.r_posix_path.p_path, dst_path) == 0);

            norns_iotask_t task;
            nornsctl_iotask_init(&task, task_op, &src, &dst);

            THEN("the norns_task structure is initialized as expected") {
                REQUIRE(task.t_id == 0);
                REQUIRE(task.t_op == task_op);
                REQUIRE(memcmp(&task.t_src, &src, 
                            sizeof(norns_resource_t)) == 0);
                REQUIRE(memcmp(&task.t_dst, &dst, 
                            sizeof(norns_resource_t)) == 0);
            }
        }
    }

    GIVEN("valid task information") {
        WHEN("initializing a task with src=NORNS_PROCESS_MEMORY and "
             "dst=NORNS_POSIX_PATH | R_REMOTE") {

            norns_op_t task_op = NORNS_IOTASK_COPY;
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            const char* dst_nsid = "tmp://";
            const char* dst_host = "node0";
            const char* dst_path = "/a/b/c";

            norns_resource_t src = NORNS_MEMORY_REGION(src_addr, src_size);

            REQUIRE(src.r_buffer.b_addr == src_addr);
            REQUIRE(src.r_buffer.b_size == src_size);

            norns_resource_t dst = NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path);

            REQUIRE(strcmp(dst.r_posix_path.p_nsid, dst_nsid) == 0);
            REQUIRE(strcmp(dst.r_posix_path.p_host, dst_host) == 0);
            REQUIRE(strcmp(dst.r_posix_path.p_path, dst_path) == 0);

            norns_iotask_t task;
            nornsctl_iotask_init(&task, task_op, &src, &dst);

            THEN("the norns_task structure is initialized as expected") {
                REQUIRE(task.t_id == 0);
                REQUIRE(task.t_op == task_op);
                REQUIRE(memcmp(&task.t_src, &src, 
                            sizeof(norns_resource_t)) == 0);
                REQUIRE(memcmp(&task.t_dst, &dst, 
                            sizeof(norns_resource_t)) == 0);
            }
        }
    }
}

SCENARIO("initialize a task with NORNSCTL_IOTASK", "[api::NORNSCTL_IOTASK]") {

    GIVEN("valid task information") {
        WHEN("initializing a task with src=NORNS_MEMORY_REGION and "
             "dst=NORNS_LOCAL_PATH") {

            norns_op_t task_op = NORNS_IOTASK_COPY;
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            const char* dst_nsid = "tmp://";
            const char* dst_path = "/a/b/c";

            norns_iotask_t task = 
                NORNSCTL_IOTASK(task_op, 
                                NORNS_MEMORY_REGION(src_addr, src_size), 
                                NORNS_LOCAL_PATH(dst_nsid, dst_path));

            THEN("the norns_task structure is initialized as expected") {
                REQUIRE(task.t_id == 0);
                REQUIRE(task.t_op == task_op);

                REQUIRE(task.t_src.r_buffer.b_addr == src_addr);

                REQUIRE(strcmp(task.t_dst.r_posix_path.p_nsid, dst_nsid) == 0);
                REQUIRE(strcmp(task.t_dst.r_posix_path.p_path, dst_path) == 0);
            }
        }

        WHEN("initializing a task with src=NORNS_PROCESS_MEMORY and "
             "dst=NORNS_REMOTE_PATH") {
            norns_op_t task_op = NORNS_IOTASK_COPY;
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            const char* dst_nsid = "tmp://";
            const char* dst_host = "node0";
            const char* dst_path = "/a/b/c";

            norns_iotask_t task = 
                NORNSCTL_IOTASK(task_op, 
                                NORNS_MEMORY_REGION(src_addr, src_size), 
                                NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path));

            THEN("the norns_task structure is initialized as expected") {
                REQUIRE(task.t_id == 0);
                REQUIRE(task.t_op == task_op);

                REQUIRE(task.t_src.r_buffer.b_addr == src_addr);

                REQUIRE(strcmp(task.t_dst.r_posix_path.p_nsid, dst_nsid) == 0);
                REQUIRE(strcmp(task.t_dst.r_posix_path.p_host, dst_host) == 0);
                REQUIRE(strcmp(task.t_dst.r_posix_path.p_path, dst_path) == 0);
            }

        }

        WHEN("initializing a task with src=NORNS_MEMORY_REGION and "
             "dst=NORNS_LOCAL_PATH") {

            norns_op_t task_op = NORNS_IOTASK_COPY;
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            const char* dst_nsid = "tmp://";
            const char* dst_path = "/a/b/c";

            norns_iotask_t task = 
                NORNSCTL_IOTASK(task_op, 
                                NORNS_MEMORY_REGION(src_addr, src_size), 
                                NORNS_SHARED_PATH(dst_nsid, dst_path));

            THEN("the norns_task structure is initialized as expected") {
                REQUIRE(task.t_id == 0);
                REQUIRE(task.t_op == task_op);

                REQUIRE(task.t_src.r_buffer.b_addr == src_addr);

                REQUIRE(strcmp(task.t_dst.r_posix_path.p_nsid, dst_nsid) == 0);
                REQUIRE(strcmp(task.t_dst.r_posix_path.p_path, dst_path) == 0);
            }
        }
    }
}

