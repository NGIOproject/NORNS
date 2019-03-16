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

#ifndef __NORNS_TYPES_H__
#define __NORNS_TYPES_H__ 1

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Types */
typedef uint32_t norns_tid_t;
typedef uint32_t norns_flags_t;
typedef int32_t  norns_status_t;
typedef int32_t  norns_error_t;

/* Resource types */
#define NORNS_PROCESS_MEMORY    0x0100000   /* Memory buffer */
#define NORNS_POSIX_PATH        0x0200000   /* POSIX path */
#define NORNS_NULL_RESOURCE     0x1000000

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
    const char* p_nsid;   /* namespace id */
    const char* p_host;   /* hostname (NULL if local) */
    const char* p_path;   /* path to "data" (i.e. file or directory) */
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

norns_resource_t
NORNS_MEMORY_REGION(void* addr, 
                    size_t size);

norns_resource_t
NORNS_LOCAL_PATH(const char* nsid, 
                 const char* path);

norns_resource_t
NORNS_REMOTE_PATH(const char* nsid, 
                  const char* host, 
                  const char* path);

norns_resource_t
NORNS_SHARED_PATH(const char* nsid, 
                  const char* path);

/* Task types */
typedef enum {
    NORNS_IOTASK_COPY   = 0x1,
    NORNS_IOTASK_MOVE   = 0x2,
    NORNS_IOTASK_REMOVE = 0x3
} norns_op_t;

/* I/O task status descriptor */
typedef struct {
    norns_status_t st_status;     /* task current status */
    norns_error_t  st_task_error; /* task return value */
    int            st_sys_errno;  /* errno returned if st_task_error == NORNS_ESYSTEM_ERROR */
    size_t         st_pending;    /* bytes pending in task */
    size_t         st_total;      /* total bytes in task */
} norns_stat_t;

/* Descriptor for an I/O task */
typedef struct {
    norns_tid_t         t_id;   /* task identifier */
    norns_op_t          t_op;   /* operation to be performed */
    norns_resource_t    t_src;  /* source resource */
    norns_resource_t    t_dst;  /* destination resource */

    /* Internal members */
    norns_stat_t        __t_status; /* cached task status */
} norns_iotask_t;

/* Additional administrative types */
typedef enum {
    NORNS_BACKEND_NVML             = 0x10000001,
    NORNS_BACKEND_LUSTRE           = 0x10000002,
    NORNS_BACKEND_ECHOFS           = 0x10000003,
    NORNS_BACKEND_POSIX_FILESYSTEM = 0x10000004
} nornsctl_backend_flags_t;

/* Administrative command IDs valid for nornsctl_send_command() */
typedef enum {
    NORNSCTL_CMD_PING = 1000,
    NORNSCTL_CMD_PAUSE_LISTEN,
    NORNSCTL_CMD_RESUME_LISTEN,
    NORNSCTL_CMD_SHUTDOWN,
} nornsctl_command_t;

#ifdef __cplusplus
};
#endif

#endif /* __NORNS_TYPES_H__ */
