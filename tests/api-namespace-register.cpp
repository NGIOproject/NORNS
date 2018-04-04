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

SCENARIO("register namespace", "[api::norns_register_namespace]") {
    GIVEN("a running urd instance") {

        test_env env;
        const char* path_b0 = env.create_directory("mnt/b0").c_str();
        const char* path_b1 = env.create_directory("mnt/b1").c_str();

        WHEN("a namespace is registered with invalid information") {

            int rv = norns_register_namespace(NULL, NULL);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a namespace is registered with an invalid nsid") {

            norns_backend_t b0 = NORNS_BACKEND(NORNS_BACKEND_NVML, path_b0, 1024);

            int rv = norns_register_namespace(NULL, &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a namespace is registered with an invalid nsid") {

            norns_backend_t b0 = NORNS_BACKEND(NORNS_BACKEND_NVML, path_b0, 1024);

            int rv = norns_register_namespace("", &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

#if 0
        WHEN("a namespace is registered with an invalid type") {

            norns_backend_t b0 = NORNS_BACKEND("b0", 42, path_b0, 1024);

            int rv = norns_register_namespace(&b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }
#endif

        WHEN("a namespace is registered with an invalid mount point") {
            norns_backend_t b0 = NORNS_BACKEND(NORNS_BACKEND_NVML, "", 1024);

            int rv = norns_register_namespace("b0://", &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a namespace is registered with an invalid mount point") {
            norns_backend_t b0 = NORNS_BACKEND(NORNS_BACKEND_NVML, NULL, 1024);

            int rv = norns_register_namespace("b0://", &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a namespace is registered with an invalid quota") {
            norns_backend_t b0 = NORNS_BACKEND(NORNS_BACKEND_NVML, path_b0, 0);

            int rv = norns_register_namespace("b0://", &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a namespace is registered with valid information") {
            norns_backend_t b0 = NORNS_BACKEND(NORNS_BACKEND_NVML, path_b0, 4096);

            int rv = norns_register_namespace("b0://", &b0);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        WHEN("attempting to register a namespace with a duplicate nsid") {
            norns_backend_t b0 = NORNS_BACKEND(NORNS_BACKEND_NVML, path_b0, 4096);
            norns_backend_t b1 = NORNS_BACKEND(NORNS_BACKEND_NVML, path_b1, 4096);

            int rv = norns_register_namespace("b0://", &b0);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_register_namespace("b0://", &b1);

            THEN("NORNS_ENAMESPACEEXISTS is returned") {
                REQUIRE(rv == NORNS_ENAMESPACEEXISTS);
            }
        }
    }

    GIVEN("a non-running urd instance") {
        WHEN("attempting to register a namespace") {

            norns_backend_t b0 = NORNS_BACKEND(NORNS_BACKEND_NVML, "mnt/foo", 1024);

            int rv = norns_register_namespace("b0://", &b0);

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
}
