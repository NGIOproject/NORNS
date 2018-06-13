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

#ifndef __NORNSCTL_LIB_H__
#define __NORNSCTL_LIB_H__ 1

#ifndef NORNSCTL_API_VERSION
#define NORNSCTL_API_VERSION 10
#endif

#include <sys/types.h>
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
    const char* b_mount;    /* mount point */
    uint32_t    b_capacity; /* namespace capacity (in megabytes) for writing data */
} norns_backend_t;

/* Descriptor for a batch job limits w.r.t. a namespace */
typedef struct {
    const char* l_nsid;     /* namespace ID */
    uint32_t    l_quota;    /* job's quota */
} norns_job_limit_t;

/* Descriptor for a batch job */
typedef struct {
    const char**        j_hosts;  /* NULL-terminated list of hostnames participating in the job */
    size_t              j_nhosts; /* entries in hostname list */
    norns_job_limit_t** j_limits; /* NULL-terminated list of limits for the job */
    size_t              j_nlimits;  /* entries in limits list */
} norns_job_t;

norns_backend_t NORNS_BACKEND(norns_flags_t flags, const char* mount_point, 
                              uint32_t capacity) __THROW;

void norns_backend_init(norns_backend_t* backend, norns_flags_t flags, 
                        const char* mount_point, uint32_t capacity) __THROW;

norns_job_limit_t NORNS_JOB_LIMIT(const char* nsid, uint32_t quota) __THROW;

void norns_job_limit_init(norns_job_limit_t* limit, const char* nsid, uint32_t quota) __THROW;

norns_job_t NORNS_JOB(const char** hosts, size_t nhosts, 
                      norns_job_limit_t** limits, size_t nlimits) __THROW;

void norns_job_init(norns_job_t* job, const char** hosts, size_t nhosts, 
                    norns_job_limit_t** limits, size_t nlimits) __THROW;

/* Check if the urd daemon is running */
norns_error_t nornsctl_ping() __THROW;

/* Send a command to the daemon (e.g. stop accepting new tasks) */
//norns_error_t norns_command();

/* Register a batch job into the system */
norns_error_t nornsctl_register_job(uint32_t jobid, norns_job_t* job) __THROW;

/* Update an existing batch job */
/* XXX: At the moment this invalidates all registered processes for this job */
norns_error_t nornsctl_update_job(uint32_t jobid, norns_job_t* job) __THROW;

/* Remove a batch job from the system */
norns_error_t nornsctl_unregister_job(uint32_t jobid) __THROW;

/* Add a process to a registered batch job */
norns_error_t nornsctl_add_process(uint32_t jobid, uid_t uid, gid_t gid, pid_t pid) __THROW;

/* Remove a process from a registered batch job */
norns_error_t nornsctl_remove_process(uint32_t jobid, uid_t uid, gid_t gid, pid_t pid) __THROW;

/* Register a namespace in the local norns server */
norns_error_t nornsctl_register_namespace(const char* nsid, norns_backend_t* backend) __THROW;

/* Update an existing namespace in the local norns server */
norns_error_t nornsctl_update_namespace(const char* nsid, norns_backend_t* backend) __THROW;

/* Unregister a namespace from the local norns server */
norns_error_t nornsctl_unregister_namespace(const char* nsid) __THROW;

/* Return a string describing the error number */
char* norns_strerror(norns_error_t errnum) __THROW;

#ifdef __cplusplus
}
#endif

#endif /* __NORNSCTL_LIB_H__ */
