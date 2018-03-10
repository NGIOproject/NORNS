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

#include <chrono>
#include <norns.h>
#include <urd.hpp>
#include <settings.hpp>

#include "fake-daemon.hpp"

//#define USE_REAL_DAEMON

SCENARIO("check request", "[api::norns_status]") {
    GIVEN("a running urd instance") {

#ifndef USE_REAL_DAEMON
        fake_daemon td;
        td.run();
#endif

        /**************************************************************************************************************/
        /* tests for error conditions                                                                                 */
        /**************************************************************************************************************/

        WHEN("checking the status of an active request") {
            
            norns_op_t task_op = NORNS_IOTASK_COPY;
            
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp0";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_PROCESS_MEMORY, NULL, 0);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_POSIX_FILESYSTEM, dst_mnt, 8192);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);
            REQUIRE(rv == NORNS_SUCCESS);
            REQUIRE(task.t_id != 0);

            norns_stat_t stats;
            rv = norns_status(&task, &stats);

            THEN("NORNS_SUCCESS is returned and task status is valid") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE((stats.st_status == NORNS_EPENDING ||
                         stats.st_status == NORNS_EINPROGRESS ||
                         stats.st_status == NORNS_EFINISHED));
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }
/*
        WHEN("submitting a request to copy data using unregistered backends") {

            norns_op_t task_op = NORNS_IOTASK_COPY;
            
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_path = "/a/b/c";

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            norns_error_t rv = norns_submit(&task);

            THEN("NORNS_ENOSUCHBACKEND is returned") {
                REQUIRE(rv == NORNS_ENOSUCHBACKEND);
            }
        }

        WHEN("submitting a request to copy data using an unregistered src backend") {

            norns_op_t task_op = NORNS_IOTASK_COPY;
            
            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp0";
            const char* src_path = "/a/b/c";
            
            const char* dst_nsid = "lustre://";
            const char* dst_mnt = "/mnt/lustre0";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t b = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 1024);

            norns_error_t rv = norns_register_backend(&cred, &b);

            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_ENOSUCHBACKEND is returned") {
                REQUIRE(rv == NORNS_ENOSUCHBACKEND);
            }

            rv = norns_unregister_backend(&cred, dst_nsid);

            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to copy data using an unregistered dst backend") {

            norns_op_t task_op = NORNS_IOTASK_COPY;
            
            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp0";
            const char* src_path = "/a/b/c";
            
            const char* dst_nsid = "lustre://";
            const char* dst_mnt = "/mnt/lustre0";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t b = NORNS_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 1024);
            norns_error_t rv = norns_register_backend(&cred, &b);

            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_ENOSUCHBACKEND is returned") {
                REQUIRE(rv == NORNS_ENOSUCHBACKEND);
            }

            rv = norns_unregister_backend(&cred, src_nsid);

            REQUIRE(rv == NORNS_SUCCESS);
        }
*/


#if 0
        /**************************************************************************************************************/
        /* tests for NORNS_IOTASK_COPY                                                                                */
        /**************************************************************************************************************/
        /* copy from process memory to .* */
        WHEN("submitting a request to copy from NORNS_PROCESS_MEMORY to NORNS_LOCAL_PATH") {
            
            norns_op_t task_op = NORNS_IOTASK_COPY;
            
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp0";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_PROCESS_MEMORY, NULL, 0);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_POSIX_FILESYSTEM, dst_mnt, 8192);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to copy from NORNS_PROCESS_MEMORY to NORNS_SHARED_PATH") {
            
            norns_op_t task_op = NORNS_IOTASK_COPY;
            
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp0";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_PROCESS_MEMORY, NULL, 0);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 8192);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);


            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to copy from NORNS_PROCESS_MEMORY to NORNS_REMOTE_PATH") {
            
            norns_op_t task_op = NORNS_IOTASK_COPY;
            
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_host = "node0";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_PROCESS_MEMORY, NULL, 0);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            /*
            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 8192);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);
            */

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                               NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* copy from local path to .* */
        WHEN("submitting a request to copy from NORNS_LOCAL_PATH to NORNS_LOCAL_PATH") {
            
            norns_op_t task_op = NORNS_IOTASK_COPY;
            
            const char* src_nsid = "tmp0://";
            const char* src_mnt = "/mnt/tmp0";
            const char* src_path = "/b/c/d";
            
            const char* dst_nsid = "tmp1://";
            const char* dst_mnt = "/mnt/tmp1";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_POSIX_FILESYSTEM, dst_mnt, 8192);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to copy from NORNS_LOCAL_PATH to NORNS_SHARED_PATH") {
            
            norns_op_t task_op = NORNS_IOTASK_COPY;
            
            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp";
            const char* src_path = "/b/c/d";
            
            const char* dst_nsid = "lustre://";
            const char* dst_mnt = "/mnt/lustre";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 8192);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to copy from NORNS_LOCAL_PATH to NORNS_REMOTE_PATH") {

            norns_op_t task_op = NORNS_IOTASK_COPY;
            
            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp";
            const char* src_path = "/b/c/d";
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp";
            const char* dst_host = "node1";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            /*
            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_NVML, dst_mnt, 8192);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);
            */

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_REMOTE_PATH to NORNS_LOCAL_PATH") {
            
            norns_op_t task_op = NORNS_IOTASK_COPY;
            
            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp";
            const char* dst_path = "/b/c/d";

            struct norns_cred cred;
            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_NVML, dst_mnt, 16384);
            norns_error_t rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path),
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_REMOTE_PATH to NORNS_SHARED_PATH") {

            norns_op_t task_op = NORNS_IOTASK_COPY;
            
            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp";
            const char* dst_path = "/b/c/d";

            struct norns_cred cred;
            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 16384);
            norns_error_t rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path),
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_REMOTE_PATH to NORNS_REMOTE_PATH") {

            norns_op_t task_op = NORNS_IOTASK_COPY;

            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "tmp://";
            const char* dst_host = "node1";
            const char* dst_path = "/b/c/d";

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path),
                                               NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path));

            norns_error_t rv = norns_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_LOCAL_PATH to NORNS_PROCESS_MEMORY") {

            norns_op_t task_op = NORNS_IOTASK_COPY;

            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "mem://";
            void* dst_addr = (void*) 0xdeadbeef;
            size_t dst_size = (size_t) 42;

            struct norns_cred cred;

            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_PROCESS_MEMORY, NULL, 0);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_MEMORY_REGION(dst_nsid, dst_addr, dst_size));

            rv = norns_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_SHARED_PATH to NORNS_PROCESS_MEMORY") {

            norns_op_t task_op = NORNS_IOTASK_COPY;

            const char* src_nsid = "lustre://";
            const char* src_mnt = "/mnt/lustre";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "mem://";
            void* dst_addr = (void*) 0xdeadbeef;
            size_t dst_size = (size_t) 42;

            struct norns_cred cred;

            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_LUSTRE, src_mnt, 16384);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_PROCESS_MEMORY, NULL, 0);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_SHARED_PATH(src_nsid, src_path), 
                                               NORNS_MEMORY_REGION(dst_nsid, dst_addr, dst_size));

            rv = norns_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_REMOTE_PATH to NORNS_PROCESS_MEMORY") {

            norns_op_t task_op = NORNS_IOTASK_COPY;

            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "mem://";
            void* dst_addr = (void*) 0xdeadbeef;
            size_t dst_size = (size_t) 42;

            struct norns_cred cred;

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_PROCESS_MEMORY, NULL, 0);
            norns_error_t rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path), 
                                               NORNS_MEMORY_REGION(dst_nsid, dst_addr, dst_size));

            rv = norns_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }


        /**************************************************************************************************************/
        /* tests for NORNS_IOTASK_MOVE                                                                                */
        /**************************************************************************************************************/
        /* move from process memory to .* */
        WHEN("submitting a request to move from NORNS_PROCESS_MEMORY to NORNS_LOCAL_PATH") {
            
            norns_op_t task_op = NORNS_IOTASK_MOVE;
            
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp0";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_PROCESS_MEMORY, NULL, 0);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_POSIX_FILESYSTEM, dst_mnt, 8192);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to move from NORNS_PROCESS_MEMORY to NORNS_SHARED_PATH") {
            
            norns_op_t task_op = NORNS_IOTASK_MOVE;
            
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp0";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_PROCESS_MEMORY, NULL, 0);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 8192);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);


            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to move from NORNS_PROCESS_MEMORY to NORNS_REMOTE_PATH") {
            
            norns_op_t task_op = NORNS_IOTASK_MOVE;
            
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_host = "node0";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_PROCESS_MEMORY, NULL, 0);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            /*
            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 8192);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);
            */

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                               NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* move from local path to .* */
        WHEN("submitting a request to move from NORNS_LOCAL_PATH to NORNS_LOCAL_PATH") {
            
            norns_op_t task_op = NORNS_IOTASK_MOVE;
            
            const char* src_nsid = "tmp0://";
            const char* src_mnt = "/mnt/tmp0";
            const char* src_path = "/b/c/d";
            
            const char* dst_nsid = "tmp1://";
            const char* dst_mnt = "/mnt/tmp1";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_POSIX_FILESYSTEM, dst_mnt, 8192);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to move from NORNS_LOCAL_PATH to NORNS_SHARED_PATH") {
            
            norns_op_t task_op = NORNS_IOTASK_MOVE;
            
            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp";
            const char* src_path = "/b/c/d";
            
            const char* dst_nsid = "lustre://";
            const char* dst_mnt = "/mnt/lustre";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 8192);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to move from NORNS_LOCAL_PATH to NORNS_REMOTE_PATH") {

            norns_op_t task_op = NORNS_IOTASK_MOVE;
            
            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp";
            const char* src_path = "/b/c/d";
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp";
            const char* dst_host = "node1";
            const char* dst_path = "/a/b/c";

            struct norns_cred cred;
            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            /*
            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_NVML, dst_mnt, 8192);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);
            */

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_REMOTE_PATH to NORNS_LOCAL_PATH") {
            
            norns_op_t task_op = NORNS_IOTASK_MOVE;
            
            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp";
            const char* dst_path = "/b/c/d";

            struct norns_cred cred;
            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_NVML, dst_mnt, 16384);
            norns_error_t rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path),
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_REMOTE_PATH to NORNS_SHARED_PATH") {

            norns_op_t task_op = NORNS_IOTASK_MOVE;
            
            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp";
            const char* dst_path = "/b/c/d";

            struct norns_cred cred;
            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 16384);
            norns_error_t rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path),
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = norns_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_REMOTE_PATH to NORNS_REMOTE_PATH") {

            norns_op_t task_op = NORNS_IOTASK_MOVE;

            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "tmp://";
            const char* dst_host = "node1";
            const char* dst_path = "/b/c/d";

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path),
                                               NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path));

            norns_error_t rv = norns_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_LOCAL_PATH to NORNS_PROCESS_MEMORY") {

            norns_op_t task_op = NORNS_IOTASK_MOVE;

            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "mem://";
            void* dst_addr = (void*) 0xdeadbeef;
            size_t dst_size = (size_t) 42;

            struct norns_cred cred;

            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_PROCESS_MEMORY, NULL, 0);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_MEMORY_REGION(dst_nsid, dst_addr, dst_size));

            rv = norns_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_SHARED_PATH to NORNS_PROCESS_MEMORY") {

            norns_op_t task_op = NORNS_IOTASK_MOVE;

            const char* src_nsid = "lustre://";
            const char* src_mnt = "/mnt/lustre";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "mem://";
            void* dst_addr = (void*) 0xdeadbeef;
            size_t dst_size = (size_t) 42;

            struct norns_cred cred;

            norns_backend_t bsrc = NORNS_BACKEND(src_nsid, NORNS_BACKEND_LUSTRE, src_mnt, 16384);
            norns_error_t rv = norns_register_backend(&cred, &bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_PROCESS_MEMORY, NULL, 0);
            rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_SHARED_PATH(src_nsid, src_path), 
                                               NORNS_MEMORY_REGION(dst_nsid, dst_addr, dst_size));

            rv = norns_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = norns_unregister_backend(&cred, src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_REMOTE_PATH to NORNS_PROCESS_MEMORY") {

            norns_op_t task_op = NORNS_IOTASK_MOVE;

            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "mem://";
            void* dst_addr = (void*) 0xdeadbeef;
            size_t dst_size = (size_t) 42;

            struct norns_cred cred;

            norns_backend_t bdst = NORNS_BACKEND(dst_nsid, NORNS_BACKEND_PROCESS_MEMORY, NULL, 0);
            norns_error_t rv = norns_register_backend(&cred, &bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNS_IOTASK(task_op, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path), 
                                               NORNS_MEMORY_REGION(dst_nsid, dst_addr, dst_size));

            rv = norns_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            rv = norns_unregister_backend(&cred, dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }
#endif

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
