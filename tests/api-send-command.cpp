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

        WHEN("a NORNSCTL_CMD_PAUSE_ACCEPT command is sent") {

            norns_error_t rv = nornsctl_send_command(NORNSCTL_CMD_PAUSE_ACCEPT, NULL);

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
                        norns_error_t rv = nornsctl_send_command(NORNSCTL_CMD_RESUME_ACCEPT, NULL);
                        REQUIRE(rv == NORNS_SUCCESS);

                        rv = norns_submit(&task);
                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(task.t_id == 1);
                    }
                }

                AND_WHEN("further NORNSCTL_CMD_PAUSE_ACCEPT commands are sent") {

                    rv = nornsctl_send_command(NORNSCTL_CMD_PAUSE_ACCEPT, NULL);

                    THEN("nornsctl_send_command() returns NORNS_SUCCESS") {
                        REQUIRE(rv == NORNS_SUCCESS);
                    }
                }
            }
        }

        WHEN("a NORNSCTL_CMD_RESUME_ACCEPT command is sent") {

            norns_error_t rv = nornsctl_send_command(NORNSCTL_CMD_RESUME_ACCEPT, NULL);

            THEN("nornsctl_send_command() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);

                AND_WHEN("further NORNSCTL_CMD_RESUME_ACCEPT commands are sent") {
                    rv = nornsctl_send_command(NORNSCTL_CMD_RESUME_ACCEPT, NULL);

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
                    rv = norns_wait(&tasks[ntasks-1]);

                    THEN("nornsctl_send_command() returns NORNS_SUCCESS") {
                        rv = nornsctl_send_command(NORNSCTL_CMD_SHUTDOWN, NULL);
                        REQUIRE(rv == NORNS_SUCCESS);
                    }
                }
            }
        }
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
