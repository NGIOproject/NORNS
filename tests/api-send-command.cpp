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
                true /* dry_run? */
            }
        );

        const char* nsid0 = "tmp0";
        bfs::path src_mnt;
        std::tie(std::ignore, src_mnt) = env.create_namespace(nsid0, "mnt/tmp0", 16384);

        WHEN("a NORNSCTL_COMMAND_PAUSE_ACCEPT command is sent") {

            norns_error_t rv = nornsctl_send_command(NORNSCTL_COMMAND_PAUSE_ACCEPT, NULL);

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
                        norns_error_t rv = nornsctl_send_command(NORNSCTL_COMMAND_RESUME_ACCEPT, NULL);
                        REQUIRE(rv == NORNS_SUCCESS);

                        rv = norns_submit(&task);
                        REQUIRE(rv == NORNS_SUCCESS);
                        REQUIRE(task.t_id == 1);
                    }
                }
            }
        }

        WHEN("a NORNSCTL_COMMAND_RESUME_ACCEPT command is sent") {

            norns_error_t rv = nornsctl_send_command(NORNSCTL_COMMAND_RESUME_ACCEPT, NULL);

            THEN("nornsctl_send_command() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        WHEN("a NORNSCTL_COMMAND_PING command is sent") {

            norns_error_t rv = nornsctl_send_command(NORNSCTL_COMMAND_PING, NULL);

            THEN("nornsctl_send_command() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        WHEN("a NORNSCTL_COMMAND_SHUTDOWN command is sent") {

            norns_error_t rv = nornsctl_send_command(NORNSCTL_COMMAND_SHUTDOWN, NULL);

            THEN("nornsctl_send_command() returns NORNS_SUCCESS") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }
    }
}
