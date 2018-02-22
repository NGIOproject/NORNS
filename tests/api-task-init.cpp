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

#include <norns.h>
#include <string.h>
#include "catch.hpp"

SCENARIO("initialize a task with norns_iotask_init", "[api::norns_iotask_init]") {

    GIVEN("invalid task information") {
        WHEN("initializing a task with NULL src and dst") {

            norns_iotask_t task;
            norns_op_t task_op = NORNS_IOTASK_COPY;

            norns_iotask_init(&task, task_op, NULL, NULL);

            THEN("task is set to 0") {
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

            norns_iotask_init(&task, task_op, NULL, &dst);

            THEN("task is set to 0") {
                norns_iotask_t dummy;
                memset(&dummy, 0, sizeof(dummy));
                REQUIRE(memcmp(&task, &dummy, sizeof(norns_iotask_t)) == 0);
            }
        }
    }

    GIVEN("invalid task information") {
        WHEN("initializing a task with a NULL dst") {

            norns_iotask_t task;
            norns_op_t task_op = NORNS_IOTASK_COPY;

            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;

            norns_resource_t src = NORNS_MEMORY_REGION(src_nsid, src_addr, src_size);

            norns_iotask_init(&task, task_op, &src, NULL);

            THEN("task is set to 0") {
                norns_iotask_t dummy;
                memset(&dummy, 0, sizeof(dummy));
                REQUIRE(memcmp(&task, &dummy, sizeof(norns_iotask_t)) == 0);
            }
        }
    }

    GIVEN("valid task information") {
        WHEN("initializing a task with src=NORNS_PROCESS_MEMORY and dst=NORNS_POSIX_PATH | R_LOCAL") {

            norns_op_t task_op = NORNS_IOTASK_COPY;
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            const char* dst_nsid = "tmp://";
            const char* dst_path = "/a/b/c";

            norns_resource_t src = NORNS_MEMORY_REGION(src_nsid, src_addr, src_size);

            REQUIRE(strcmp(src.r_nsid, src_nsid) == 0);
            REQUIRE(src.r_buffer.b_addr == src_addr);
            REQUIRE(src.r_buffer.b_size == src_size);

            norns_resource_t dst = NORNS_LOCAL_PATH(dst_nsid, dst_path);

            REQUIRE(strcmp(dst.r_nsid, dst_nsid) == 0);
            REQUIRE(dst.r_posix_path.p_host == NULL);
            REQUIRE(strcmp(dst.r_posix_path.p_path, dst_path) == 0);

            norns_iotask_t task;
            norns_iotask_init(&task, task_op, &src, &dst);

            THEN("the norns_task structure is initialized as expected") {
                REQUIRE(task.t_id == 0);
                REQUIRE(task.t_op == task_op);
                REQUIRE(memcmp(&task.t_src, &src, sizeof(norns_resource_t)) == 0);
                REQUIRE(memcmp(&task.t_dst, &dst, sizeof(norns_resource_t)) == 0);
            }
        }
    }

    GIVEN("valid task information") {
        WHEN("initializing a task with src=NORNS_PROCESS_MEMORY and dst=NORNS_POSIX_PATH | R_REMOTE") {

            norns_op_t task_op = NORNS_IOTASK_COPY;
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            const char* dst_nsid = "tmp://";
            const char* dst_host = "node0";
            const char* dst_path = "/a/b/c";

            norns_resource_t src = NORNS_MEMORY_REGION(src_nsid, src_addr, src_size);

            REQUIRE(strcmp(src.r_nsid, src_nsid) == 0);
            REQUIRE(src.r_buffer.b_addr == src_addr);
            REQUIRE(src.r_buffer.b_size == src_size);

            norns_resource_t dst = NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path);

            REQUIRE(strcmp(dst.r_nsid, dst_nsid) == 0);
            REQUIRE(strcmp(dst.r_posix_path.p_host, dst_host) == 0);
            REQUIRE(strcmp(dst.r_posix_path.p_path, dst_path) == 0);

            norns_iotask_t task;
            norns_iotask_init(&task, task_op, &src, &dst);

            THEN("the norns_task structure is initialized as expected") {
                REQUIRE(task.t_id == 0);
                REQUIRE(task.t_op == task_op);
                REQUIRE(memcmp(&task.t_src, &src, sizeof(norns_resource_t)) == 0);
                REQUIRE(memcmp(&task.t_dst, &dst, sizeof(norns_resource_t)) == 0);
            }
        }
    }
}

SCENARIO("initialize a task with NORNS_TASK", "[api::NORNS_TASK]") {

    GIVEN("valid task information") {
        WHEN("initializing a task with src=NORNS_MEMORY_REGION and dst=NORNS_LOCAL_PATH") {

            norns_op_t task_op = NORNS_IOTASK_COPY;
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            const char* dst_nsid = "tmp://";
            const char* dst_path = "/a/b/c";

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                           NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                           NORNS_LOCAL_PATH(dst_nsid, dst_path));

            THEN("the norns_task structure is initialized as expected") {
                REQUIRE(task.t_id == 0);
                REQUIRE(task.t_op == task_op);

                REQUIRE(strcmp(task.t_src.r_nsid, src_nsid) == 0);
                REQUIRE(task.t_src.r_buffer.b_addr == src_addr);

                REQUIRE(strcmp(task.t_dst.r_nsid, dst_nsid) == 0);
                REQUIRE(strcmp(task.t_dst.r_posix_path.p_path, dst_path) == 0);
            }
        }

        WHEN("initializing a task with src=NORNS_PROCESS_MEMORY and dst=NORNS_REMOTE_PATH") {
            norns_op_t task_op = NORNS_IOTASK_COPY;
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            const char* dst_nsid = "tmp://";
            const char* dst_host = "node0";
            const char* dst_path = "/a/b/c";

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                           NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                           NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path));

            THEN("the norns_task structure is initialized as expected") {
                REQUIRE(task.t_id == 0);
                REQUIRE(task.t_op == task_op);

                REQUIRE(strcmp(task.t_src.r_nsid, src_nsid) == 0);
                REQUIRE(task.t_src.r_buffer.b_addr == src_addr);

                REQUIRE(strcmp(task.t_dst.r_nsid, dst_nsid) == 0);
                REQUIRE(strcmp(task.t_dst.r_posix_path.p_host, dst_host) == 0);
                REQUIRE(strcmp(task.t_dst.r_posix_path.p_path, dst_path) == 0);
            }

        }

        WHEN("initializing a task with src=NORNS_MEMORY_REGION and dst=NORNS_LOCAL_PATH") {

            norns_op_t task_op = NORNS_IOTASK_COPY;
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            const char* dst_nsid = "tmp://";
            const char* dst_path = "/a/b/c";

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                           NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                           NORNS_SHARED_PATH(dst_nsid, dst_path));

            THEN("the norns_task structure is initialized as expected") {
                REQUIRE(task.t_id == 0);
                REQUIRE(task.t_op == task_op);

                REQUIRE(strcmp(task.t_src.r_nsid, src_nsid) == 0);
                REQUIRE(task.t_src.r_buffer.b_addr == src_addr);

                REQUIRE(strcmp(task.t_dst.r_nsid, dst_nsid) == 0);
                REQUIRE(strcmp(task.t_dst.r_posix_path.p_path, dst_path) == 0);
            }
        }
    }
}

