/* * Copyright (C) 2017-2018 Barcelona Supercomputing Center
 *                           Centro Nacional de Supercomputacion
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

#ifndef NORNS_API_VERSION
#define NORNS_API_VERSION 10
#endif

#include <sys/types.h>
#include <stdint.h>
#include <assert.h>

#include "norns_types.h"
#include "norns_error.h"
#include "norns_backends.h"
#include "norns_resources.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Process credentials */
struct norns_cred {
    // TODO: to be completed, but at least...
    pid_t cr_pid;    /* PID of the process */
    gid_t cr_gid;    /* GID of the process */
};

/* Descriptor for an I/O task */
typedef struct {
    norns_tid_t         t_id;   /* task identifier */
    norns_op_t          t_op;   /* operation to be performed */
    norns_resource_t    t_src;  /* source resource */
    norns_resource_t    t_dst;  /* destination resource */
} norns_iotask_t;

#if 0 // deprecated
#define NORNS_IOTD_INIT(type, src, dst) \
{ \
    .io_taskid = 0, \
    .io_optype = (type), \
    .io_src = src, \
    .io_dst = dst \
}
#endif

/* Task types */
//enum {
//    NORNS_COPY   = 00000000,
//    NORNS_MOVE   = 00000001,
//    NORNS_LOCAL  = 00000010,
//    NORNS_REMOTE = 00000020
//};


#define NORNS_IOTASK_COPY 0x1
#define NORNS_IOTASK_MOVE 0x2


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

/**************************************************************************/
/* Client API                                                             */
/**************************************************************************/

/* Initialize an asynchronous I/O task */
void norns_iotask_init(norns_iotask_t* task, norns_op_t operation,
                       norns_resource_t* src, norns_resource_t* dst);

norns_iotask_t NORNS_IOTASK(norns_op_t operation, norns_resource_t src, 
                            norns_resource_t dst);

// XXX deprecated
/* Initialize a resource */
//void norns_resource_init(norns_resource_flags_t flags, norns_resource_t* res, 
//                         void* info);

// XXX deprecated
/* Submit an asynchronous I/O task */
// int norns_transfer(struct norns_iotd* iotdp) __THROW;

/* Submit an asynchronous I/O task */
norns_error_t norns_submit(norns_iotask_t* task) __THROW;

/* wait for the completion of the task associated to task */
int norns_wait(norns_iotask_t* task) __THROW;

/* Try to cancel an asynchronous I/O task associated with task */
ssize_t norns_cancel(norns_iotask_t* task) __THROW;

/* Retrieve return status associated with task */
ssize_t norns_return(norns_iotask_t* task, struct norns_iotst* statp) __THROW;

/* Retrieve current status associated with task (if task is NULL, retrieve status for all running tasks) */
ssize_t norns_progress(norns_iotask_t* task, struct norns_iotst* statp) __THROW;

/* Retrieve error status associated with task */
int norns_error(norns_iotask_t* task) __THROW;

/* Check if the urd daemon is running */
int norns_ping() __THROW;


/**************************************************************************/
/* Administrative API                                                     */
/* (only authenticated processes will be able to successfully use these)  */
/**************************************************************************/

/* Storage backend descriptor */
typedef struct {
    const char* b_nsid;     /* namespace ID for this backend (e.g. nvm01, tmpfs02, ...) */
    int         b_type;     /* backend type */
    const char* b_mount;    /* mount point */
    size_t      b_quota;    /* backend capacity (in megabytes) allocated to the job for writing */
} norns_backend_t;

norns_backend_t NORNS_BACKEND(const char* nsid, norns_flags_t flags, 
                              const char* mount_point, uint32_t quota);

void norns_backend_init(norns_backend_t* backend, const char* nsid, 
                        norns_flags_t flags, const char* mount_point,
                        uint32_t quota);


/* Batch job descriptor */
typedef struct {
    const char**      j_hosts;  /* NULL-terminated list of hostnames participating in the job */
    size_t            j_nhosts; /* entries in hostname list */
    norns_backend_t** j_backends; /* NULL-terminated list of storage backends the job is allowed to use */
    size_t            j_nbackends; /* entries in backend list */
} norns_job_t;

norns_job_t NORNS_JOB(const char** hosts, size_t nhosts, 
                      norns_backend_t** backends, size_t nbackends);

void norns_job_init(norns_job_t* job, const char** hosts, size_t nhosts, 
                    norns_backend_t** backends, size_t nbackends);

/* Send a command to the daemon (e.g. stop accepting new tasks) */
int norns_command(struct norns_cred* auth);

/* Register a batch job into the system */
int norns_register_job(struct norns_cred* auth, uint32_t jobid, norns_job_t* job);

/* Update an existing batch job */
/* XXX: At the moment this invalidates all registered processes for this job */
int norns_update_job(struct norns_cred* auth, uint32_t jobid, norns_job_t* job);

/* Remove a batch job from the system */
int norns_unregister_job(struct norns_cred* auth, uint32_t jobid);

/* Add a process to a registered batch job */
int norns_add_process(struct norns_cred* auth, uint32_t jobid, uid_t uid, gid_t gid, pid_t pid);

/* Remove a process from a registered batch job */
int norns_remove_process(struct norns_cred* auth, uint32_t jobid, uid_t uid, gid_t gid, pid_t pid);

/* Register a backend in the local norns server */
int norns_register_backend(struct norns_cred* auth, norns_backend_t* backend);

/* Update an existing backend in the local norns server */
int norns_register_backend(struct norns_cred* auth, norns_backend_t* backend);

/* Unregister a backend from the local norns server */
int norns_unregister_backend(struct norns_cred* auth, const char* prefix);


char* norns_strerror(int errnum);

#ifdef __cplusplus
}
#endif

#endif /* __NORNS_LIB_H__ */
