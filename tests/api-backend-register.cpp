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
#include "catch.hpp"
#include "fake-daemon.hpp"

// enable to test connections with an already running daemon
//#define USE_REAL_DAEMON

SCENARIO("register backend", "[api::norns_register_backend]") {
    GIVEN("a running urd instance") {

#ifndef USE_REAL_DAEMON
        fake_daemon td;
        td.run();
#endif

        WHEN("a backend is registered with invalid information") {

            struct norns_cred cred;

            int rv = norns_register_backend(&cred, NULL);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a backend is registered with an invalid prefix") {

            struct norns_cred cred;
            norns_backend_t b0 = NORNS_BACKEND(NULL, NORNS_BACKEND_NVML, "/mnt/b0", 1024);

            int rv = norns_register_backend(&cred, &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a backend is registered with an invalid prefix") {

            struct norns_cred cred;
            norns_backend_t b0 = NORNS_BACKEND("", NORNS_BACKEND_NVML, "/mnt/b0", 1024);

            int rv = norns_register_backend(&cred, &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

#if 0
        WHEN("a backend is registered with an invalid type") {

            struct norns_cred cred;
            norns_backend_t b0 = NORNS_BACKEND("b0", 42, "/mnt/b0", 1024);

            int rv = norns_register_backend(&cred, &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }
#endif

        WHEN("a backend is registered with an invalid mount point") {
            struct norns_cred cred;
            norns_backend_t b0 = NORNS_BACKEND("b0://", NORNS_BACKEND_NVML, "", 1024);

            int rv = norns_register_backend(&cred, &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a backend is registered with an invalid mount point") {
            struct norns_cred cred;
            norns_backend_t b0 = NORNS_BACKEND("b0://", NORNS_BACKEND_NVML, NULL, 1024);

            int rv = norns_register_backend(&cred, &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a backend is registered with an invalid quota") {
            struct norns_cred cred;
            norns_backend_t b0 = NORNS_BACKEND("b0://", NORNS_BACKEND_NVML, "/mnt/b0", 0);

            int rv = norns_register_backend(&cred, &b0);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a backend is registered with valid information") {
            struct norns_cred cred;
            norns_backend_t b0 = NORNS_BACKEND("b0://", NORNS_BACKEND_NVML, "/mnt/b0", 4096);

            int rv = norns_register_backend(&cred, &b0);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        WHEN("attempting to register a backend with a duplicate prefix") {
            struct norns_cred cred;
            norns_backend_t b0 = NORNS_BACKEND("b0://", NORNS_BACKEND_NVML, "/mnt/b0", 4096);
            norns_backend_t b1 = NORNS_BACKEND("b0://", NORNS_BACKEND_NVML, "/mnt/b1", 4096);

            int rv = norns_register_backend(&cred, &b0);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_register_backend(&cred, &b1);

            THEN("NORNS_EBACKENDEXISTS is returned") {
                REQUIRE(rv == NORNS_EBACKENDEXISTS);
            }
        }

#ifndef USE_REAL_DAEMON
        int ret = td.stop();
        REQUIRE(ret == 0);
#endif

    }

    GIVEN("a non-running urd instance") {
        WHEN("attempting to register a backend") {

            struct norns_cred cred;
            norns_backend_t b0 = NORNS_BACKEND("b0://", NORNS_BACKEND_NVML, "/mnt/b0", 1024);

            int rv = norns_register_backend(&cred, &b0);

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
}
