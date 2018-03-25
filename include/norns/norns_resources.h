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

#ifndef __NORNS_RESOURCES_H__
#define __NORNS_RESOURCES_H__ 1

#include <sys/types.h>
#include <stdint.h>     /* For uint32_t et al. */

#ifdef __cplusplus
extern "C" {
#endif

/* Resource types */
#define NORNS_PROCESS_MEMORY    0x0100000   /* Memory buffer */
#define NORNS_POSIX_PATH        0x0200000   /* POSIX path */

/* Access types */
#define R_LOCAL         0x0000010    /* Local resource (default) */
#define R_REMOTE        0x0000020    /* Remote resource */
#define R_SHARED        0x0000040    /* Shared resource */

/* Descriptor for a memory region */
typedef struct {
    void*                   b_addr;     /* base memory address */
    size_t                  b_size;     /* region size */
} norns_memory_region_t;

/* Descriptor for a POSIX path */
typedef struct {
    const char* p_nsid;  /* namespace id */
    const char* p_host;  /* hostname (NULL if local) */
    const char* p_path;  /* path to "data" (i.e. file or directory) */
} norns_posix_path_t;

/* Data resource descriptor */
typedef struct {

    // options:
    // - read from local nvm and write to lustre
    // - read from lustre and write to local nvm
    // - read from remote nvm and write to local nvm
    // - read from local nvm and write to remote nvm
    // - read from process memory and write to local nvm
    // - read from process memory and write to lustre 
    // - echofs: "read" from lustre into echofs
    // - echofs: "write" from echofs to lustre
    //
    //
    // - NEXTGenIO input resources: 
    // 1.   local nvm       nvm://path/to/dir/[file]                DAX-NVML
    // 2.   local tmpfs     tmpfs://path/to/dir/[file]              DAX-NVML
    // 3.   lustre          lustre://path/to/dir/[file]             POSIX
    // 4.   remote nvm      nvm@hostname://path/to/dir/[file]       DAX-NVML+RDMA/TCP
    // 5.   echofs          echofs://path/to/dir/[file]             CUSTOM
    // 6.   process memory  [pointer + size]                        MEMORY
    //
    // - NEXTGenIO output resources:
    // 1.   local nvm       nvm://path/to/dir/[file]                DAX-NVML
    // 2.   local tmpfs     tmpfs://path/to/dir/[file]              DAX-NVML
    // 3.   lustre (path)   lustre://path/to/dir/[file]             POSIX
    // 4.   remote nvm      nvm@hostname:://path/to/dir/[file]      DAX-NVML+RDMA/TCP
    // 5.   echofs          echofs://path/to/dir/[file]             CUSTOM

    norns_flags_t r_flags; /* resource type and flags */
    union {
        norns_memory_region_t r_buffer;
        norns_posix_path_t r_posix_path;
    };
} norns_resource_t;

norns_resource_t NORNS_MEMORY_REGION(void* addr, size_t size);
norns_resource_t NORNS_LOCAL_PATH(const char* nsid, const char* path);
norns_resource_t NORNS_REMOTE_PATH(const char* nsid, const char* host, const char* path);
norns_resource_t NORNS_SHARED_PATH(const char* nsid, const char* path);

#ifdef __cplusplus
}
#endif

#endif /* __NORNS_RESOURCES_H__ */
