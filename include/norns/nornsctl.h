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

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************/
/* Control API                                                            */
/* (only processes with enough permissions to access the control socket   */
/* will be able to successfully use these)                                */
/**************************************************************************/

/* Descriptor for a storage backend */
typedef struct {
    const char* b_nsid;     /* namespace ID for this backend (e.g. nvm01, tmpfs02, ...) */
    int         b_type;     /* backend type */
    const char* b_mount;    /* mount point */
    size_t      b_quota;    /* backend capacity (in megabytes) for writing data */
} norns_backend_t;

/* Descriptor for a batch job */
typedef struct {
    const char**      j_hosts;  /* NULL-terminated list of hostnames participating in the job */
    size_t            j_nhosts; /* entries in hostname list */
    norns_backend_t** j_backends; /* NULL-terminated list of storage backends the job is allowed to use */
    size_t            j_nbackends; /* entries in backend list */
} norns_job_t;

/* Descriptor for a batch job access rights */
typedef struct {
    const char* r_nsid;     /* namespace ID */
    size_t      r_quota;    /* job's quota */
} norns_access_rights_t;

norns_backend_t NORNS_BACKEND(const char* nsid, norns_flags_t flags, 
                              const char* mount_point, uint32_t quota) __THROW;

void norns_backend_init(norns_backend_t* backend, const char* nsid, 
                        norns_flags_t flags, const char* mount_point,
                        uint32_t quota) __THROW;

norns_job_t NORNS_JOB(const char** hosts, size_t nhosts, 
                      norns_backend_t** backends, size_t nbackends) __THROW;

void norns_job_init(norns_job_t* job, const char** hosts, size_t nhosts, 
                    norns_backend_t** backends, size_t nbackends) __THROW;

/* Check if the urd daemon is running */
norns_error_t norns_ping() __THROW;

/* Send a command to the daemon (e.g. stop accepting new tasks) */
//norns_error_t norns_command();

/* Register a batch job into the system */
norns_error_t norns_register_job(uint32_t jobid, norns_job_t* job) __THROW;

/* Update an existing batch job */
/* XXX: At the moment this invalidates all registered processes for this job */
norns_error_t norns_update_job(uint32_t jobid, norns_job_t* job) __THROW;

/* Remove a batch job from the system */
norns_error_t norns_unregister_job(uint32_t jobid) __THROW;

/* Add a process to a registered batch job */
norns_error_t norns_add_process(uint32_t jobid, uid_t uid, gid_t gid, pid_t pid) __THROW;

/* Remove a process from a registered batch job */
norns_error_t norns_remove_process(uint32_t jobid, uid_t uid, gid_t gid, pid_t pid) __THROW;

/* Register a backend in the local norns server */
norns_error_t norns_register_backend(norns_backend_t* backend) __THROW;

/* Update an existing backend in the local norns server */
norns_error_t norns_register_backend(norns_backend_t* backend) __THROW;

/* Unregister a backend from the local norns server */
norns_error_t norns_unregister_backend(const char* prefix) __THROW;

/* Return a string describing the error number */
char* norns_strerror(norns_error_t errnum) __THROW;

#ifdef __cplusplus
}
#endif

#endif /* __NORNSCTL_LIB_H__ */
