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
