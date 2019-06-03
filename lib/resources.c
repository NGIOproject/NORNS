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

#include <unistd.h>
#include <string.h>

#include "norns.h"
#include "log.h"

#if 0
/* Initialize a norns_resource structure according to the flags and 
 * parameters passed */
void 
norns_resource_init(norns_resource_flags_t flags, norns_resource_t* res, 
                    void* info) {

    if(res == NULL || info == NULL) {
        return;
    }

    memset(res, 0, sizeof(*res));

    /* resource is a memory buffer */
    if(flags & NORNS_PROCESS_MEMORY) {
        res->r_buffer = *((norns_memory_region_t*) info);
        return;
    }

    /* resource is a POSIX path */
    if(flags & NORNS_POSIX_PATH) {

        res->r_posix_path = *((norns_posix_path_t*) info);

        // ensure that R_LOCAL and R_SHARED have a NULL hostname
        if((flags & R_LOCAL) || (flags & R_SHARED)) {
            res->r_posix_path.p_host = NULL;
        }
        return;
    }
}
#endif

inline norns_resource_t
NORNS_MEMORY_REGION(void* addr, size_t size) {

    norns_resource_t res;

    res.r_flags = NORNS_PROCESS_MEMORY;
    res.r_buffer.b_addr = addr;
    res.r_buffer.b_size = size;

    return res;
}

inline norns_resource_t
NORNS_LOCAL_PATH(const char* nsid, const char* path) {

    norns_resource_t res;

    res.r_flags = NORNS_POSIX_PATH | R_LOCAL;
    res.r_posix_path.p_nsid = nsid;
    res.r_posix_path.p_host = NULL;
    res.r_posix_path.p_path = path;

    return res;
}

inline norns_resource_t
NORNS_REMOTE_PATH(const char* nsid, const char* host, const char* path) {

    norns_resource_t res;

    res.r_flags = NORNS_POSIX_PATH | R_REMOTE;
    res.r_posix_path.p_nsid = nsid;
    res.r_posix_path.p_host = host;
    res.r_posix_path.p_path = path;

    return res;
}

inline norns_resource_t
NORNS_SHARED_PATH(const char* nsid, const char* path) {

    norns_resource_t res;

    res.r_flags = NORNS_POSIX_PATH | R_SHARED;
    res.r_posix_path.p_nsid = nsid;
    res.r_posix_path.p_host = NULL;
    res.r_posix_path.p_path = path;

    return res;
}
