/* * Copyright (C) 2017 Barcelona Supercomputing Center
 *                    Centro Nacional de Supercomputacion
 *
 * This file is part of the Data Scheduler, a daemon for tracking and managing
 * requests for asynchronous data transfer in a hierarchical storage environment.
 *
 * See AUTHORS file in the top level directory for information
 * regarding developers and contributors.
 *
 * The Data Scheduler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Data Scheduler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Data Scheduler.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __NORNS_LIB_H__
#define __NORNS_LIB_H__ 1

#include "norns_error.h"
#include "norns_backends.h"

#include <sys/types.h>
#include <stdint.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Process credentials */
struct norns_cred {
    // TODO: to be completed, but at least...
    pid_t cr_pid;    /* PID of the process */
    gid_t cr_gid;    /* GID of the process */
};

struct norns_membuf {
    void*  b_addr;      /* memory address */
    size_t b_size;      /* memory size */
};

#define NORNS_MEMBUFFER_INIT(addr, size) \
{ \
    .b_addr = (addr), \
    .b_size = (size) \
}

struct norns_path {
    const char* p_hostname;     /* hostname (NULL if local) */
    const char* p_datapath;     /* path to "data" (i.e. file or directory) */
};

#define NORNS_LOCAL_PATH_INIT(path) \
{ \
    .p_hostname = NULL, \
    .p_datapath = (path) \
}

#define NORNS_REMOTE_PATH_INIT(hostname, path) \
{ \
    .p_hostname = (hostname), \
    .p_datapath = (path) \
}

/* Input data resource descriptor  */
struct norns_data_in {

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

    uint32_t    in_type;         /* type of resource */
    union {
        struct norns_membuf __in_buffer;
        struct norns_path __in_path;
    } __in_location;

#define in_buffer __in_location.__in_buffer
#define in_path __in_location.__in_path
};

/* Output data resource descriptor  */
struct norns_data_out {

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
    // - input resources: 
    // 1.   local nvm       nvm://path/to/dir/[file]
    // 2.   lustre          lustre://path/to/dir/[file]
    // 3.   remote nvm      nvm@hostname://path/to/dir/[file]
    // 4.   echofs          echofs://path/to/dir/[file]
    // 5.   process memory  [pointer + size]
    //
    // - output resources:
    // 1.   local nvm (path)    nvm://foobar
    // 2.   lustre (path)       lustre://foobar
    // 3.   remote nvm          nvm@hostname:://foobar
    // 4.   echofs              echofs://foobar

    uint32_t    out_type;         /* type of resource */
    struct norns_path out_path;
};


/* I/O task descriptor */
struct norns_iotd {
    uint32_t            io_taskid;     /* task identifier */
    uint32_t            io_optype;    /* operation to be performed */

    struct norns_data_in io_src;   /* data source */
    struct norns_data_out io_dst;   /* data destination */
//    struct norns_cred*  ni_auth;   /* process credentials (NULL if unprivileged) */
};

#define NORNS_IOTD_INIT(type, src, dst) \
{ \
    .io_taskid = 0, \
    .io_optype = (type), \
    .io_src = src, \
    .io_dst = dst \
}

#define NORNS_INPUT_PATH_INIT(type, path) \
{ \
    .in_type = (type), \
    .__in_location = { \
        .__in_path = path \
    } \
}

#define NORNS_INPUT_BUFFER_INIT(addr, size) \
{ \
    .in_type = NORNS_BACKEND_PROCESS_MEMORY, \
    .__in_location = { \
        .__in_buffer = NORNS_MEMBUFFER_INIT((addr), (size)) \
    } \
}

#define NORNS_OUTPUT_PATH_INIT(type, path) \
{ \
    .out_type = (type), \
    .out_path = path \
}




/* Task types */
//enum {
//    NORNS_COPY   = 00000000,
//    NORNS_MOVE   = 00000001,
//    NORNS_LOCAL  = 00000010,
//    NORNS_REMOTE = 00000020
//};

#define NORNS_COPY      00000000
#define NORNS_MOVE      00000001
#define NORNS_LOCAL     00000010
#define NORNS_REMOTE    00000020


/* I/O task status descriptor */
struct norns_iotst {
    int ni_status;  /* current status */
};

/* Status codes */
enum {
    NORNS_WAITING,
    NORNS_INPROGRESS,
    NORNS_COMPLETE
};


//void norns_init() __THROW;

//int norns_getconf() __THROW;

/**************************************************************************/
/* Client API                                                             */
/**************************************************************************/

/* Enqueue an asynchronous I/O task */
int norns_transfer(struct norns_iotd* iotdp) __THROW;

/* wait for the completion of the task associated to iotdp */
int norns_wait(struct norns_iotd* iotdp) __THROW;

/* Try to cancel an asynchronous I/O task associated with iotdp */
ssize_t norns_cancel(struct norns_iotd* iotdp) __THROW;

/* Retrieve return status associated with iotdp */
ssize_t norns_return(struct norns_iotd* iotdp, struct norns_iotst* statp) __THROW;

/* Retrieve current status associated with iotdp (if iotdp is NULL, retrieve status for all running tasks) */
ssize_t norns_progress(struct norns_iotd* iotdp, struct norns_iotst* statp) __THROW;

/* Retrieve error status associated with iotdp */
int norns_error(struct norns_iotd* iotdp) __THROW;


/**************************************************************************/
/* Administrative API                                                     */
/* (only authenticated processes will be able to successfully call these) */
/**************************************************************************/

/* Storage backend descriptor */
struct norns_backend {
    int         b_type;
    const char* b_prefix; /* prefix ID for this backend (e.g. nvm01, tmpfs02, ...) */
    const char* b_mount; /* mount point */
    size_t      b_quota; /* backend capacity (in megabytes) allocated to the job for writing */
};

#define NORNS_BACKEND_INIT(type, prefix, mount, quota) \
{ \
    .b_type = (type), \
    .b_prefix = (prefix), \
    .b_mount = (mount), \
    .b_quota = (quota) \
}

#define NORNS_ALLOC(size)       \
({                              \
    size_t __n = (size);        \
    void* __p = malloc(__n);    \
    assert(__p != NULL);        \
    __p;                        \
})

#define NORNS_FREE(p)   \
({                      \
    free(__p;)          \
})

#define NORNS_PLIST_ALLOC(type, size)                               \
({                                                                  \
    size_t __n = (size);                                            \
    void** __plist = (void**) malloc(sizeof(type) * (__n + 1));     \
    memset(__plist, 0, __n + 1);                                    \
    (type*) __plist;                                                \
})

#define NORNS_PLIST_FREE(plist)     \
({                                  \
    free((plist));                  \
})

/* Batch job descriptor */
struct norns_job {
    const char**            jb_hosts;  /* NULL-terminated list of hostnames participating in the job */
    size_t                  jb_nhosts; /* entries in hostname list */
    struct norns_backend**  jb_backends; /* NULL-terminated list of storage backends the job is allowed to use */
    size_t                  jb_nbackends; /* entries in backend list */
};

#define NORNS_JOB_INIT(hosts, nhosts, backends, nbackends) \
{ \
    .jb_hosts = (hosts), \
    .jb_nhosts = (nhosts), \
    .jb_backends = (backends), \
    .jb_nbackends = (nbackends) \
}


/* Send a command to the daemon (e.g. stop accepting new tasks) */
int norns_command(struct norns_cred* auth);

/* Register a batch job into the system */
int norns_register_job(struct norns_cred* auth, uint32_t jobid, struct norns_job* job);

/* Update an existing batch job */
/* XXX: At the moment this invalidates all registered processes for this job */
int norns_update_job(struct norns_cred* auth, uint32_t jobid, struct norns_job* job);

/* Remove a batch job from the system */
int norns_unregister_job(struct norns_cred* auth, uint32_t jobid);

/* Add a process to a registered batch job */
int norns_add_process(struct norns_cred* auth, uint32_t jobid, uid_t uid, gid_t gid, pid_t pid);

/* Remove a process from a registered batch job */
int norns_remove_process(struct norns_cred* auth, uint32_t jobid, uid_t uid, gid_t gid, pid_t pid);


char* norns_strerror(int errnum);

#ifdef __cplusplus
}
#endif

#endif /* __NORNS_LIB_H__ */
