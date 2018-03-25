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
#include <string.h>
#include "catch.hpp"

SCENARIO("initialize resources", "[api::norns_resource_init]") {
    GIVEN("valid resource information") {
        WHEN("initializing a memory buffer resource") {

            void* addr = (void*) 0xdeadbeef;
            size_t size = (size_t) 42;

            norns_resource_t res = NORNS_MEMORY_REGION(addr, size);
                
            THEN("the norns_resource structure is initialized as expected") {
                REQUIRE(res.r_flags == NORNS_PROCESS_MEMORY);
                REQUIRE(res.r_buffer.b_addr == addr);
                REQUIRE(res.r_buffer.b_size == size);
            }
        }

        WHEN("initializing a local posix file resource") {

            const char* nsid = "tmp://";
            const char* path = "/a/b/c";

            norns_resource_t res = NORNS_LOCAL_PATH(nsid, path);

            THEN("the norns_resource structure is initialized as a local file") {
                REQUIRE(res.r_flags == (NORNS_POSIX_PATH | R_LOCAL));
                REQUIRE(strcmp(res.r_posix_path.p_nsid, nsid) == 0);
                REQUIRE(res.r_posix_path.p_host == NULL);
                REQUIRE(strcmp(res.r_posix_path.p_path, path) == 0);
            }
        }

        WHEN("initializing a shared posix file resource") {

            const char* nsid = "tmp://";
            const char* path = "/a/b/c";

            norns_resource_t res = NORNS_SHARED_PATH(nsid, path);

            THEN("the norns_resource structure is initialized as a local file") {
                REQUIRE(res.r_flags == (NORNS_POSIX_PATH | R_SHARED));
                REQUIRE(strcmp(res.r_posix_path.p_nsid, nsid) == 0);
                REQUIRE(res.r_posix_path.p_host == NULL);
                REQUIRE(strcmp(res.r_posix_path.p_path, path) == 0);
            }
        }

        WHEN("initializing a remote posix file resource") {

            const char* nsid = "tmp://";
            const char* host = "node42";
            const char* path = "/a/b/c";

            norns_resource_t res = NORNS_REMOTE_PATH(nsid, host, path);

            THEN("the norns_resource structure is initialized as a remote file") {
                REQUIRE(res.r_flags == (NORNS_POSIX_PATH | R_REMOTE));
                REQUIRE(strcmp(res.r_posix_path.p_nsid, nsid) == 0);
                REQUIRE(strcmp(res.r_posix_path.p_host, host) == 0);
                REQUIRE(strcmp(res.r_posix_path.p_path, path) == 0);
            }
        }
    }
}
