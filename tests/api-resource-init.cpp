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

#include <string.h>
#include "norns.h"
#include "nornsctl.h"
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
