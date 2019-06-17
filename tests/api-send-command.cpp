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
#include "nornsctl.h"
#include "test-env.hpp"
#include "catch.hpp"

SCENARIO("send control commands to urd", "[api::nornsctl_send_command]") {
    GIVEN("a running urd instance") {

        test_env env(
            fake_daemon_cfg {
                true, /* dry_run? */
                500000 /* dry_run_duration */
            }
        );

        const char* nsid0 = "tmp0";
        bfs::path src_mnt;
        std::tie(std::ignore, src_mnt) = env.create_namespace(nsid0, "mnt/tmp0", 16384);

        WHEN("a NORNSCTL_CMD_PAUSE_LISTEN command is sent") {

            norns_error_t rv = nornsctl_send_command(NORNSCTL_CMD_PAUSE_LISTEN, NULL);

            THEN("nornsctl_send_command() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);

                AND_THEN("norns_submit() returns NORNS_EACCEPTPAUSED") {
                    norns_iotask_t task = 
                        NORNS_IOTASK(NORNS_IOTASK_COPY, 
                                    NORNS_MEMORY_REGION((void*)0xdeadbeef, 42), 
                                    NORNS_LOCAL_PATH(nsid0, "foobar"));

                    rv = norns_submit(&task);
                    REQUIRE(rv == NORNS_EACCEPTPAUSED);

                    AND_THEN("nornsctl_send_command() returns NORNS_SUCCESS and norns_submit() succeeds") {
                        norns_error_t rv = nornsctl_send_command(NORNSCTL_CMD_RESUME_LISTEN, NULL);
                        REQUIRE(rv == NORNS_SUCCESS);

                        rv = norns_submit(&task);
                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(task.t_id == 1);
                    }
                }

                AND_WHEN("further NORNSCTL_CMD_PAUSE_LISTEN commands are sent") {

                    rv = nornsctl_send_command(NORNSCTL_CMD_PAUSE_LISTEN, NULL);

                    THEN("nornsctl_send_command() returns NORNS_SUCCESS") {
                        REQUIRE(rv == NORNS_SUCCESS);
                    }
                }
            }
        }

        WHEN("a NORNSCTL_CMD_RESUME_LISTEN command is sent") {

            norns_error_t rv = nornsctl_send_command(NORNSCTL_CMD_RESUME_LISTEN, NULL);

            THEN("nornsctl_send_command() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);

                AND_WHEN("further NORNSCTL_CMD_RESUME_LISTEN commands are sent") {
                    rv = nornsctl_send_command(NORNSCTL_CMD_RESUME_LISTEN, NULL);

                    THEN("nornsctl_send_command() returns NORNS_SUCCESS") {
                        REQUIRE(rv == NORNS_SUCCESS);
                    }
                }
            }
        }

        WHEN("a NORNSCTL_CMD_PING command is sent") {

            norns_error_t rv = nornsctl_send_command(NORNSCTL_CMD_PING, NULL);

            THEN("nornsctl_send_command() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

#ifndef USE_REAL_DAEMON
        WHEN("a NORNSCTL_CMD_SHUTDOWN command is sent and there are no pending tasks") {

            norns_error_t rv = nornsctl_send_command(NORNSCTL_CMD_SHUTDOWN, NULL);

            THEN("nornsctl_send_command() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }
#endif

        WHEN("a NORNSCTL_CMD_SHUTDOWN command is sent and there are pending tasks") {

            const size_t ntasks = 10;
            norns_iotask_t tasks[ntasks];

            const auto submit_task = [&] {

                norns_iotask_t task = 
                    NORNS_IOTASK(NORNS_IOTASK_COPY, 
                                 NORNS_MEMORY_REGION((void*)0xdeadbeef, 43), 
                                 NORNS_LOCAL_PATH(nsid0, "foobar"));
                auto rv = norns_submit(&task);

                return task;
            };

            for(size_t i=0; i<ntasks; ++i) {
                tasks[i] = submit_task();
            }

            norns_error_t rv = nornsctl_send_command(NORNSCTL_CMD_SHUTDOWN, NULL);

            THEN("nornsctl_send_command() returns NORNS_ETASKSPENDING") {
                REQUIRE(rv == NORNS_ETASKSPENDING);

                AND_WHEN("all tasks complete") {
                    rv = nornsctl_wait(&tasks[ntasks-1], NULL);

                    THEN("nornsctl_send_command() returns NORNS_SUCCESS") {
                        rv = nornsctl_send_command(NORNSCTL_CMD_SHUTDOWN, NULL);
                        REQUIRE(rv == NORNS_SUCCESS);
                    }
                }
            }
        }

        env.notify_success();
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("checking the status of all requests") {
            norns_error_t rv = nornsctl_send_command(NORNSCTL_CMD_PING, NULL);

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
#endif
}
