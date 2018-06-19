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

SCENARIO("register namespace", "[api::nornsctl_register_namespace]") {
    GIVEN("a running urd instance") {

        test_env env;
        const bfs::path path_b0 = env.create_directory("mnt/b0", env.basedir()); 
        const bfs::path path_b1 = env.create_directory("mnt/b0", env.basedir()); 

        WHEN("a namespace is registered with invalid information") {

            int rv = nornsctl_register_namespace(NULL, NULL);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a namespace is registered with an invalid nsid") {

            nornsctl_backend_t b0 = 
                NORNSCTL_BACKEND(NORNS_BACKEND_NVML, path_b0.c_str(), 1024);

            int rv = nornsctl_register_namespace(NULL, &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a namespace is registered with an invalid nsid") {

            nornsctl_backend_t b0 = 
                NORNSCTL_BACKEND(NORNS_BACKEND_NVML, path_b0.c_str(), 1024);

            int rv = nornsctl_register_namespace("", &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

#if 0
        WHEN("a namespace is registered with an invalid type") {

            nornsctl_backend_t b0 = NORNSCTL_BACKEND("b0", 42, path_b0, 1024);

            int rv = nornsctl_register_namespace(&b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }
#endif

        WHEN("a namespace is registered with an invalid mount point") {
            nornsctl_backend_t b0 = NORNSCTL_BACKEND(NORNS_BACKEND_NVML, "", 1024);

            int rv = nornsctl_register_namespace("b0://", &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a namespace is registered with an invalid mount point") {
            nornsctl_backend_t b0 = NORNSCTL_BACKEND(NORNS_BACKEND_NVML, NULL, 1024);

            int rv = nornsctl_register_namespace("b0://", &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a namespace is registered with an invalid quota") {
            nornsctl_backend_t b0 = 
                NORNSCTL_BACKEND(NORNS_BACKEND_NVML, path_b0.c_str(), 0);

            int rv = nornsctl_register_namespace("b0://", &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a namespace is registered with valid information") {
            nornsctl_backend_t b0 = 
                NORNSCTL_BACKEND(NORNS_BACKEND_NVML, path_b0.c_str(), 4096);

            int rv = nornsctl_register_namespace("b0://", &b0);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        WHEN("attempting to register a namespace with a duplicate nsid") {
            nornsctl_backend_t b0 = 
                NORNSCTL_BACKEND(NORNS_BACKEND_NVML, path_b0.c_str(), 4096);
            nornsctl_backend_t b1 = 
                NORNSCTL_BACKEND(NORNS_BACKEND_NVML, path_b1.c_str(), 4096);

            int rv = nornsctl_register_namespace("b0://", &b0);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_register_namespace("b0://", &b1);

            THEN("NORNS_ENAMESPACEEXISTS is returned") {
                REQUIRE(rv == NORNS_ENAMESPACEEXISTS);
            }
        }

        env.notify_success();
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("attempting to register a namespace") {

            nornsctl_backend_t b0 = NORNSCTL_BACKEND(NORNS_BACKEND_NVML, "mnt/foo", 1024);

            int rv = nornsctl_register_namespace("b0://", &b0);

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
#endif
}
