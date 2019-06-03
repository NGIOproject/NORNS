#include "nornsctl.h"
#include "test-env.hpp"
#include "catch.hpp"

SCENARIO("check control request", "[api::nornsctl_error]") {
    GIVEN("a running urd instance") {

        test_env env(
            fake_daemon_cfg {
                true /* dry_run? */
            }
        );

        const char* nsid0 = "tmp0";
        const char* nsid1 = "tmp1";
        bfs::path src_mnt, dst_mnt;

        // create namespaces
        std::tie(std::ignore, src_mnt) = 
            env.create_namespace(nsid0, "mnt/tmp0", 16384);
        std::tie(std::ignore, dst_mnt) = 
            env.create_namespace(nsid1, "mnt/tmp1", 16384);

        // define input names
        std::vector<int> input_data(100, 42);
        void* src_buf = input_data.data();
        size_t src_buf_size = input_data.size() * sizeof(int);
        const bfs::path src_file = "/a/b/c/file";
        size_t src_file_size = 4096;

        // define output names
        const bfs::path dst_file = "/b/c/d/file";

        // create input data
        env.add_to_namespace(nsid0, "/a/b/c/file", 4096);

        /**********************************************************************/
        /* tests for error conditions                                         */
        /**********************************************************************/

        WHEN("checking the status of an active request") {
            
            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_MEMORY_REGION(src_buf, src_buf_size), 
                                NORNS_LOCAL_PATH(nsid1, dst_file.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

                norns_stat_t stats;
                rv = nornsctl_error(&task, &stats);

                THEN("NORNS_SUCCESS is returned and task status is valid") {
                    REQUIRE(rv == NORNS_SUCCESS);
                    REQUIRE((stats.st_status == NORNS_EPENDING ||
                            stats.st_status == NORNS_EINPROGRESS ||
                            stats.st_status == NORNS_EFINISHED));
                }
            }
        }

        WHEN("checking the status of an active request") {
            
            norns_iotask_t task = 
                NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                NORNS_LOCAL_PATH(nsid0, src_file.c_str()), 
                                NORNS_LOCAL_PATH(nsid1, dst_file.c_str()));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("nornsctl_submit() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);

retry:
                norns_stat_t stats;
                rv = nornsctl_error(&task, &stats);

                THEN("NORNS_SUCCESS is returned and task status is valid") {
                    REQUIRE(rv == NORNS_SUCCESS);
                    REQUIRE((stats.st_status == NORNS_EPENDING ||
                            stats.st_status == NORNS_EINPROGRESS ||
                            stats.st_status == NORNS_EFINISHED));
                }

                if(stats.st_status != NORNS_EFINISHED)
                    goto retry;
            }
        }

        env.notify_success();
/*
        WHEN("submitting a request to copy data using unregistered backends") {

            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_path = "/a/b/c";

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_MEMORY_REGION(src_addr, src_size), 
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("NORNS_ENOSUCHBACKEND is returned") {
                REQUIRE(rv == NORNS_ENOSUCHBACKEND);
            }
        }

        WHEN("submitting a request to copy data using an unregistered src backend") {

            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp0";
            const char* src_path = "/a/b/c";
            
            const char* dst_nsid = "lustre://";
            const char* dst_mnt = "/mnt/lustre0";
            const char* dst_path = "/a/b/c";

            nornsctl_backend_t ns = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 1024);

            norns_error_t rv = nornsctl_register_namespace(&b);

            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_ENOSUCHBACKEND is returned") {
                REQUIRE(rv == NORNS_ENOSUCHBACKEND);
            }

            rv = nornsctl_unregister_namespace(dst_nsid);

            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to copy data using an unregistered dst backend") {

            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp0";
            const char* src_path = "/a/b/c";
            
            const char* dst_nsid = "lustre://";
            const char* dst_mnt = "/mnt/lustre0";
            const char* dst_path = "/a/b/c";

            nornsctl_backend_t ns = NORNSCTL_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 1024);
            norns_error_t rv = nornsctl_register_namespace(&b);

            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_ENOSUCHBACKEND is returned") {
                REQUIRE(rv == NORNS_ENOSUCHBACKEND);
            }

            rv = nornsctl_unregister_namespace(src_nsid);

            REQUIRE(rv == NORNS_SUCCESS);
        }
*/


#if 0
        /**************************************************************************************************************/
        /* tests for NORNS_IOTASK_COPY                                                                                */
        /**************************************************************************************************************/
        /* copy from process memory to .* */
        WHEN("submitting a request to copy from NORNS_PROCESS_MEMORY to NORNS_LOCAL_PATH") {
            
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp0";
            const char* dst_path = "/a/b/c";

            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_POSIX_FILESYSTEM, dst_mnt, 8192);
            norns_error_t rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to copy from NORNS_PROCESS_MEMORY to NORNS_SHARED_PATH") {
            
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp0";
            const char* dst_path = "/a/b/c";

            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 8192);
            norns_error_t rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);


            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to copy from NORNS_PROCESS_MEMORY to NORNS_REMOTE_PATH") {
            
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_host = "node0";
            const char* dst_path = "/a/b/c";

            /*
            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 8192);
            norns_error_t rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);
            */

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                               NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* copy from local path to .* */
        WHEN("submitting a request to copy from NORNS_LOCAL_PATH to NORNS_LOCAL_PATH") {
            
            const char* src_nsid = "tmp0://";
            const char* src_mnt = "/mnt/tmp0";
            const char* src_path = "/b/c/d";
            
            const char* dst_nsid = "tmp1://";
            const char* dst_mnt = "/mnt/tmp1";
            const char* dst_path = "/a/b/c";

            nornsctl_backend_t bsrc = NORNSCTL_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = nornsctl_register_namespace(&bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_POSIX_FILESYSTEM, dst_mnt, 8192);
            rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to copy from NORNS_LOCAL_PATH to NORNS_SHARED_PATH") {
            
            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp";
            const char* src_path = "/b/c/d";
            
            const char* dst_nsid = "lustre://";
            const char* dst_mnt = "/mnt/lustre";
            const char* dst_path = "/a/b/c";

            nornsctl_backend_t bsrc = NORNSCTL_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = nornsctl_register_namespace(&bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 8192);
            rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to copy from NORNS_LOCAL_PATH to NORNS_REMOTE_PATH") {

            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp";
            const char* src_path = "/b/c/d";
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp";
            const char* dst_host = "node1";
            const char* dst_path = "/a/b/c";

            nornsctl_backend_t bsrc = NORNSCTL_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = nornsctl_register_namespace(&bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            /*
            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_NVML, dst_mnt, 8192);
            rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);
            */

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_REMOTE_PATH to NORNS_LOCAL_PATH") {
            
            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp";
            const char* dst_path = "/b/c/d";

            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_NVML, dst_mnt, 16384);
            norns_error_t rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path),
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_REMOTE_PATH to NORNS_SHARED_PATH") {

            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp";
            const char* dst_path = "/b/c/d";

            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 16384);
            norns_error_t rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path),
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_REMOTE_PATH to NORNS_REMOTE_PATH") {

            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "tmp://";
            const char* dst_host = "node1";
            const char* dst_path = "/b/c/d";

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path),
                                               NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_LOCAL_PATH to NORNS_PROCESS_MEMORY") {

            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "mem://";
            void* dst_addr = (void*) 0xdeadbeef;
            size_t dst_size = (size_t) 42;


            nornsctl_backend_t bsrc = NORNSCTL_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = nornsctl_register_namespace(&bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_MEMORY_REGION(dst_nsid, dst_addr, dst_size));

            rv = nornsctl_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_SHARED_PATH to NORNS_PROCESS_MEMORY") {

            const char* src_nsid = "lustre://";
            const char* src_mnt = "/mnt/lustre";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "mem://";
            void* dst_addr = (void*) 0xdeadbeef;
            size_t dst_size = (size_t) 42;


            nornsctl_backend_t bsrc = NORNSCTL_BACKEND(src_nsid, NORNS_BACKEND_LUSTRE, src_mnt, 16384);
            norns_error_t rv = nornsctl_register_namespace(&bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_SHARED_PATH(src_nsid, src_path), 
                                               NORNS_MEMORY_REGION(dst_nsid, dst_addr, dst_size));

            rv = nornsctl_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to copy from NORNS_REMOTE_PATH to NORNS_PROCESS_MEMORY") {

            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "mem://";
            void* dst_addr = (void*) 0xdeadbeef;
            size_t dst_size = (size_t) 42;

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path), 
                                               NORNS_MEMORY_REGION(dst_nsid, dst_addr, dst_size));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }


        /**************************************************************************************************************/
        /* tests for NORNS_IOTASK_MOVE                                                                                */
        /**************************************************************************************************************/
        /* move from process memory to .* */
        WHEN("submitting a request to move from NORNS_PROCESS_MEMORY to NORNS_LOCAL_PATH") {
            
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp0";
            const char* dst_path = "/a/b/c";

            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_POSIX_FILESYSTEM, dst_mnt, 8192);
            norns_error_t rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                               NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to move from NORNS_PROCESS_MEMORY to NORNS_SHARED_PATH") {
            
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp0";
            const char* dst_path = "/a/b/c";

            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 8192);
            rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);


            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                               NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to move from NORNS_PROCESS_MEMORY to NORNS_REMOTE_PATH") {
            
            const char* src_nsid = "mem://";
            void* src_addr = (void*) 0xdeadbeef;
            size_t src_size = (size_t) 42;
            
            const char* dst_nsid = "tmp://";
            const char* dst_host = "node0";
            const char* dst_path = "/a/b/c";

            /*
            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 8192);
            rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);
            */

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                               NORNS_MEMORY_REGION(src_nsid, src_addr, src_size), 
                                               NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* move from local path to .* */
        WHEN("submitting a request to move from NORNS_LOCAL_PATH to NORNS_LOCAL_PATH") {
            
            const char* src_nsid = "tmp0://";
            const char* src_mnt = "/mnt/tmp0";
            const char* src_path = "/b/c/d";
            
            const char* dst_nsid = "tmp1://";
            const char* dst_mnt = "/mnt/tmp1";
            const char* dst_path = "/a/b/c";

            nornsctl_backend_t bsrc = NORNSCTL_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = nornsctl_register_namespace(&bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_POSIX_FILESYSTEM, dst_mnt, 8192);
            rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to move from NORNS_LOCAL_PATH to NORNS_SHARED_PATH") {
            
            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp";
            const char* src_path = "/b/c/d";
            
            const char* dst_nsid = "lustre://";
            const char* dst_mnt = "/mnt/lustre";
            const char* dst_path = "/a/b/c";

            nornsctl_backend_t bsrc = NORNSCTL_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = nornsctl_register_namespace(&bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 8192);
            rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        WHEN("submitting a request to move from NORNS_LOCAL_PATH to NORNS_REMOTE_PATH") {

            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp";
            const char* src_path = "/b/c/d";
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp";
            const char* dst_host = "node1";
            const char* dst_path = "/a/b/c";

            nornsctl_backend_t bsrc = NORNSCTL_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = nornsctl_register_namespace(&bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            /*
            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_NVML, dst_mnt, 8192);
            rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);
            */

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(task.t_id != 0);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_REMOTE_PATH to NORNS_LOCAL_PATH") {
            
            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";
            
            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp";
            const char* dst_path = "/b/c/d";

            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_NVML, dst_mnt, 16384);
            norns_error_t rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path),
                                               NORNS_LOCAL_PATH(dst_nsid, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_REMOTE_PATH to NORNS_SHARED_PATH") {

            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "tmp://";
            const char* dst_mnt = "/mnt/tmp";
            const char* dst_path = "/b/c/d";

            nornsctl_backend_t bdst = NORNSCTL_BACKEND(dst_nsid, NORNS_BACKEND_LUSTRE, dst_mnt, 16384);
            norns_error_t rv = nornsctl_register_namespace(&bdst);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path),
                                               NORNS_SHARED_PATH(dst_nsid, dst_path));

            rv = nornsctl_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using a remote node as source is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_REMOTE_PATH to NORNS_REMOTE_PATH") {

            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "tmp://";
            const char* dst_host = "node1";
            const char* dst_path = "/b/c/d";

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path),
                                               NORNS_REMOTE_PATH(dst_nsid, dst_host, dst_path));

            norns_error_t rv = nornsctl_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_LOCAL_PATH to NORNS_PROCESS_MEMORY") {

            const char* src_nsid = "tmp://";
            const char* src_mnt = "/mnt/tmp";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "mem://";
            void* dst_addr = (void*) 0xdeadbeef;
            size_t dst_size = (size_t) 42;


            nornsctl_backend_t bsrc = NORNSCTL_BACKEND(src_nsid, NORNS_BACKEND_NVML, src_mnt, 16384);
            norns_error_t rv = nornsctl_register_namespace(&bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                               NORNS_LOCAL_PATH(src_nsid, src_path), 
                                               NORNS_MEMORY_REGION(dst_nsid, dst_addr, dst_size));

            rv = nornsctl_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_SHARED_PATH to NORNS_PROCESS_MEMORY") {

            const char* src_nsid = "lustre://";
            const char* src_mnt = "/mnt/lustre";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "mem://";
            void* dst_addr = (void*) 0xdeadbeef;
            size_t dst_size = (size_t) 42;


            nornsctl_backend_t bsrc = NORNSCTL_BACKEND(src_nsid, NORNS_BACKEND_LUSTRE, src_mnt, 16384);
            norns_error_t rv = nornsctl_register_namespace(&bsrc);
            REQUIRE(rv == NORNS_SUCCESS);

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                               NORNS_SHARED_PATH(src_nsid, src_path), 
                                               NORNS_MEMORY_REGION(dst_nsid, dst_addr, dst_size));

            rv = nornsctl_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            // cleanup
            rv = nornsctl_unregister_namespace(src_nsid);
            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }

        /* using the process memory as destination is not allowed (yet) */
        WHEN("submitting a request to move from NORNS_REMOTE_PATH to NORNS_PROCESS_MEMORY") {

            const char* src_nsid = "tmp://";
            const char* src_host = "node0";
            const char* src_path = "/a/b/c";

            const char* dst_nsid = "mem://";
            void* dst_addr = (void*) 0xdeadbeef;
            size_t dst_size = (size_t) 42;

            norns_iotask_t task = NORNSCTL_IOTASK(NORNS_IOTASK_MOVE, 
                                               NORNS_REMOTE_PATH(src_nsid, src_host, src_path), 
                                               NORNS_MEMORY_REGION(dst_nsid, dst_addr, dst_size));

            rv = nornsctl_submit(&task);

            THEN("NORNS_ENOTSUPPORTED is returned") {
                REQUIRE(rv == NORNS_ENOTSUPPORTED);
            }

            rv = nornsctl_unregister_namespace(dst_nsid);
            REQUIRE(rv == NORNS_SUCCESS);
        }
#endif

        env.notify_success();
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("attempting to request a transfer") {

            norns_iotask_t task = 
                NORNSCTL_IOTASK(
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


SCENARIO("check requests", "[api::nornsctl_status]") {
    GIVEN("a running urd instance") {

        test_env env(
            fake_daemon_cfg {
                true /* dry_run? */
            }
        );

        const char* nsid0 = "tmp0";
        const char* nsid1 = "tmp1";
        bfs::path src_mnt, dst_mnt;

        // create namespaces
        std::tie(std::ignore, src_mnt) = 
            env.create_namespace(nsid0, "mnt/tmp0", 16384);
        std::tie(std::ignore, dst_mnt) = 
            env.create_namespace(nsid1, "mnt/tmp1", 16384);

        // define input names
        void* src_buf;
        size_t src_buf_size __attribute__((unused));
        const bfs::path src_file = "/a/b/c/file";
        size_t src_file_size = 4096;

        // define output names
        const bfs::path dst_file = "/b/c/d/file";

        // create input data
        env.add_to_namespace(nsid0, src_file, 4096);

        // 64MiB buffer
        std::vector<int> input_data(16*1024*1024, 42);
        src_buf = input_data.data();
        src_buf_size = input_data.size() * sizeof(int);

        WHEN("checking the status of all requests without actual requests running") {
            nornsctl_stat_t global_stats;
            norns_error_t rv = nornsctl_status(&global_stats);
            REQUIRE(rv == NORNS_SUCCESS);
            REQUIRE(global_stats.st_pending_tasks == 0);
            REQUIRE(global_stats.st_running_tasks == 0);
            REQUIRE(global_stats.st_eta == 0.0);
        }

        WHEN("checking the status of all requests") {
            
            const size_t ntasks = 10;
            norns_iotask_t tasks[ntasks];
            size_t sizes[] = 
                { 4*1024*1024, 16*1024*1024, 32*1024*1024, 64*1024*1024 };

            for(size_t i=0; i<ntasks; ++i) {

                tasks[i] =
                    NORNSCTL_IOTASK(NORNS_IOTASK_COPY, 
                                 NORNS_MEMORY_REGION(src_buf, sizes[i%4]), 
                                 NORNS_LOCAL_PATH(nsid1, dst_file.c_str()));

                norns_error_t rv = nornsctl_submit(&tasks[i]);
                REQUIRE(rv == NORNS_SUCCESS);
                REQUIRE(tasks[i].t_id == i + 1);
            }

            THEN("The status returned is consistent with status of the tasks "
                 "sent") {
                bool finished_tasks[ntasks];
                memset(finished_tasks, 0, sizeof(finished_tasks));
                size_t tasks_remaining = ntasks;
                size_t tasks_completed = 0;

                do {
                    nornsctl_stat_t global_stats;
                    norns_error_t rv = nornsctl_status(&global_stats);
                    REQUIRE(rv == NORNS_SUCCESS);
                    REQUIRE((global_stats.st_pending_tasks + 
                             global_stats.st_running_tasks) <= ntasks);

                    for(size_t i=0; i<ntasks; ++i) {
                        if(!finished_tasks[i]) {
                            norns_stat_t stats;
                            nornsctl_error(&tasks[i], &stats);

                            if(stats.st_status == NORNS_EFINISHED ||
                               stats.st_status == NORNS_EFINISHEDWERROR) {
                                finished_tasks[i] = true;
                                --tasks_remaining;
                                ++tasks_completed;
                            }
                        }
                    }

                    std::this_thread::sleep_for(std::chrono::microseconds(500));
                }
                while(tasks_remaining != 0);

                THEN("The ETA returned is 0 when no tasks are left") {
                    nornsctl_stat_t global_stats;
                    norns_error_t rv = nornsctl_status(&global_stats);
                    REQUIRE(rv == NORNS_SUCCESS);
                    REQUIRE(global_stats.st_pending_tasks == 0);
                    REQUIRE(global_stats.st_running_tasks == 0);
                    REQUIRE(global_stats.st_eta == 0.0);
                }
            }
        }

        env.notify_success();
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("checking the status of all requests") {
            nornsctl_stat_t global_stats;
            norns_error_t rv = nornsctl_status(&global_stats);

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
#endif
}
