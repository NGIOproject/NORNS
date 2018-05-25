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

SCENARIO("unregister namespace", "[api::norns_unregister_namespace]") {
    GIVEN("a running urd instance") {

        test_env env;
        const bfs::path path_b0 = env.create_directory("mnt/b0", env.basedir());

        WHEN("a namespace is unregistered with an invalid prefix") {

            int rv = norns_unregister_namespace(NULL);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("attempting to unregister a non-existing namespace") {

            int rv = norns_unregister_namespace("b0://");

            THEN("NORNS_ENOSUCHNAMESPACE is returned") {
                REQUIRE(rv == NORNS_ENOSUCHNAMESPACE);
            }
        }

        WHEN("unregistering a registered namespace") {

            const char* nsid = "b0://";

            norns_backend_t b0 = 
                NORNS_BACKEND(NORNS_BACKEND_NVML, path_b0.c_str(), 4096);

            int rv = norns_register_namespace(nsid, &b0);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_namespace(nsid);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
            }

        }

        env.notify_success();
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("attempting to unregister a namespace") {

            int rv = norns_unregister_namespace("b0://");

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
#endif
}
