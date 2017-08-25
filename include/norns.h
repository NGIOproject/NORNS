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

#include <features.h>
#include <sys/types.h>
#include <stdint.h>
#include <assert.h>

__BEGIN_DECLS

/* Error codes */
#define NORNS_SUCCESS           0
#define NORNS_EBADPARAMS        -1
#define NORNS_ENOMEM            -2
#define NORNS_ECONNFAILED       -3
#define NORNS_ERPCSENDFAILED    -4

typedef uint32_t jobid_t;

/* Process credentials */
struct norns_cred {
    // TODO: to be completed, but at least...
    pid_t cr_pid;    /* PID of the process */
    gid_t cr_gid;    /* GID of the process */
};

/* Data resource descriptor  */
struct norns_resource {
    const char* r_hostname;     /* hostname */
    const char* r_path;         /* path to "data" (i.e. file or directory) */
    uint32_t    r_type;         /* type of resource */
};


/* I/O task descriptor */
struct norns_iotd {
    uint32_t            ni_tid;     /* task identifier */

    struct norns_resource ni_src;   /* data source */
    struct norns_resource ni_dst;   /* data destination */
    uint32_t            ni_type;    /* operation to be performed */
    struct norns_cred*  ni_auth;   /* process credentials (NULL if unprivileged) */

    /* Internal members. */
    pid_t       __pid;      /* pid of the process that made the request */
    uint32_t    __jobid;    /* job id of the process that made the request (XXX Slurm dependent)*/
                        
};

/* Storage resource types */
#define NORNS_NVML      0x1000
#define NORNS_POSIX     0x1001



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

#define NORNS_LOCAL_NVML    0x10000000
#define NORNS_REMOTE_NVML   0x10000001
#define NORNS_LUSTRE        0x10000002

/* Storage backend descriptor */
struct norns_backend {
    int         b_type;
    const char* b_mount; /* mount point */
    size_t      b_quota; /* backend capacity (in megabytes) allocated to the job */

};

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
    uint32_t                jb_jobid; /* desired job ID (for later requests) */
    const char**            jb_hosts;  /* NULL-terminated list of hostnames participating in the job */
    size_t                  jb_nhosts; /* entries in hostname list */
    struct norns_backend**  jb_backends; /* NULL-terminated list of storage backends the job will use */
    size_t                  jb_nbackends; /* entries in backend list */
};


/* Send a command to the daemon (e.g. stop accepting new tasks) */
int norns_command(struct norns_cred* auth);

/* Register and describe a batch job */
int norns_register_job(struct norns_cred* auth, struct norns_job* job);

/* Update the description of an existing batch job */
int norns_update_job(struct norns_cred* auth, struct norns_job* job);

/* Remove the description of a batch job */
int norns_remove_job(struct norns_cred* auth, struct norns_job* job);



__END_DECLS

#endif /* __NORNS_LIB_H__ */
