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

#ifndef __NORNSCTL_LIB_H__
#define __NORNSCTL_LIB_H__ 1

#ifndef NORNSCTL_API_VERSION
#define NORNSCTL_API_VERSION 10
#endif

#include <stdint.h>     /* For uint32_t et al. */
#include <stdbool.h>    /* For bool */
#include <time.h>       /* For struct timespec */

#include "norns_types.h"
#include "norns_error.h"

#ifdef __NORNS_DEBUG__
#include "norns_debug.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************/
/* Control API                                                            */
/* (only processes with enough permissions to access the control socket   */
/* will be able to successfully use these)                                */
/**************************************************************************/

/* Descriptor for a storage namespace */
typedef struct {
    int         b_type;     /* namespace type */
    bool        b_track;    /* should the service track its contents? */
    const char* b_mount;    /* mount point */
    uint32_t    b_capacity; /* namespace capacity (in megabytes) for writing data */
} nornsctl_backend_t;

/* Descriptor for a batch job limits w.r.t. a namespace */
typedef struct {
    const char* l_nsid;     /* namespace ID */
    uint32_t    l_quota;    /* job's quota */
} nornsctl_job_limit_t;

/* Descriptor for a batch job */
typedef struct {
    const char**           j_hosts;  /* NULL-terminated list of hostnames participating in the job */
    size_t                 j_nhosts; /* entries in hostname list */
    nornsctl_job_limit_t** j_limits; /* NULL-terminated list of limits for the job */
    size_t                 j_nlimits;  /* entries in limits list */
} nornsctl_job_t;

/* Global task status descriptor */
typedef struct {
    /* Number of running tasks */
    size_t st_running_tasks;

    /* Number of pending tasks */
    size_t st_pending_tasks;

    /* E.T.A. in seconds for running tasks or NAN if estimation
     * is not yet available
     *
     * This value is computed as the max of alls ETAs for all currently 
     * running tasks */
    double st_eta;
} nornsctl_stat_t;

nornsctl_backend_t 
NORNSCTL_BACKEND(nornsctl_backend_flags_t flags, 
                 bool track,
                 const char* mount_point, 
                 uint32_t capacity) __THROW;

void 
nornsctl_backend_init(nornsctl_backend_t* backend,
                      nornsctl_backend_flags_t flags,
                      bool track,
                      const char* mount_point,
                      uint32_t capacity) __THROW;

nornsctl_job_limit_t 
NORNSCTL_JOB_LIMIT(const char* nsid, 
                   uint32_t quota) __THROW;

void 
nornsctl_job_limit_init(nornsctl_job_limit_t* limit, 
                        const char* nsid, 
                        uint32_t quota) __THROW;

nornsctl_job_t 
NORNSCTL_JOB(const char** hosts, 
             size_t nhosts, 
             nornsctl_job_limit_t** limits, 
             size_t nlimits) __THROW;

void 
nornsctl_job_init(nornsctl_job_t* job, 
                  const char** hosts, 
                  size_t nhosts, 
                  nornsctl_job_limit_t** limits, 
                  size_t nlimits) __THROW;

/* Send a command to the service daemon */
norns_error_t
nornsctl_send_command(nornsctl_command_t command, 
                      void* args) __THROW;

norns_error_t
nornsctl_status(nornsctl_stat_t* stats) __THROW;

/* Register a batch job into the system */
norns_error_t 
nornsctl_register_job(uint32_t jobid, 
                      nornsctl_job_t* job) __THROW;

/* Update an existing batch job */
/* XXX: At the moment this invalidates all registered processes for this job */
norns_error_t 
nornsctl_update_job(uint32_t jobid, 
                    nornsctl_job_t* job) __THROW;

/* Remove a batch job from the system */
norns_error_t 
nornsctl_unregister_job(uint32_t jobid) __THROW;

/* Add a process to a registered batch job */
norns_error_t 
nornsctl_add_process(uint32_t jobid, 
                     uid_t uid, 
                     gid_t gid, 
                     pid_t pid) __THROW;

/* Remove a process from a registered batch job */
norns_error_t 
nornsctl_remove_process(uint32_t jobid, 
                        uid_t uid, 
                        gid_t gid, 
                        pid_t pid) __THROW;

/* Register a namespace in the local norns server */
norns_error_t 
nornsctl_register_namespace(const char* nsid, 
                            nornsctl_backend_t* backend) __THROW;

/* Update an existing namespace in the local norns server */
norns_error_t 
nornsctl_update_namespace(const char* nsid, 
                          nornsctl_backend_t* backend) __THROW;

/* Unregister a namespace from the local norns server */
norns_error_t 
nornsctl_unregister_namespace(const char* nsid) __THROW;


/* Initialize an asynchronous I/O task */
void 
nornsctl_iotask_init(norns_iotask_t* task, 
                     norns_op_t operation,
                     norns_resource_t* src, 
                     norns_resource_t* dst) __THROW;
norns_iotask_t 
NORNSCTL_IOTASK(norns_op_t operation, 
                norns_resource_t src, ...) __THROW;


/* Submit an asynchronous I/O task */
norns_error_t 
nornsctl_submit(norns_iotask_t* task) __THROW;

/* wait for the completion of the I/O task associated to 'task' */
norns_error_t 
nornsctl_wait(norns_iotask_t* task,
              const struct timespec* timeout) __THROW;

/* Try to cancel an asynchronous I/O task associated with task */
norns_error_t 
nornsctl_cancel(norns_iotask_t* task) __THROW;

/* Check the status of a submitted I/O task */
norns_error_t 
nornsctl_error(norns_iotask_t* task, 
               norns_stat_t* stats) __THROW;

/* Return a string describing the error number */
char* 
nornsctl_strerror(norns_error_t errnum) __THROW;

#ifdef __cplusplus
}
#endif

#endif /* __NORNSCTL_LIB_H__ */
