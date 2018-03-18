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

#include <stdio.h>
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "norns.h"
#include "nornsctl.h"

#include "xmalloc.h"
#include "daemon-communication.h"

static bool validate_job(norns_job_t* job);
static bool validate_backend(norns_backend_t* backend);

/* Control API */
int
norns_ping() {
    return send_ping_request();
}

/* Register and describe a batch job */
int 
norns_register_job(struct norns_cred* auth, uint32_t jobid, 
                   norns_job_t* job) {

    if(auth == NULL || !validate_job(job)) {
        return NORNS_EBADARGS;
    }

    return send_job_request(NORNS_JOB_REGISTER, auth, jobid, job);
}

/* Update an existing batch job */
int 
norns_update_job(struct norns_cred* auth, uint32_t jobid, 
                 norns_job_t* job) {

    if(auth == NULL || !validate_job(job)) {
        return NORNS_EBADARGS;
    }

    return send_job_request(NORNS_JOB_UPDATE, auth, jobid, job);
}


/* Remove a batch job from the system */
int norns_unregister_job(struct norns_cred* auth, uint32_t jobid) {

    if(auth == NULL) {
        return NORNS_EBADARGS;
    }

    return send_job_request(NORNS_JOB_UNREGISTER, auth, jobid, NULL);
}


/* Add a process to a registered batch job */
int 
norns_add_process(struct norns_cred* auth, uint32_t jobid, uid_t uid,
                  gid_t gid, pid_t pid) {

    if(auth == NULL) {
        return NORNS_EBADARGS;
    }

    return send_process_request(NORNS_PROCESS_ADD, auth, jobid, uid, gid, pid);
}


/* Remove a process from a registered batch job */
int 
norns_remove_process(struct norns_cred* auth, uint32_t jobid, uid_t uid,
                     gid_t gid, pid_t pid) {

    if(auth == NULL) {
        return NORNS_EBADARGS;
    }

    return send_process_request(NORNS_PROCESS_REMOVE, auth, jobid, 
                                uid, gid, pid);
}

/* Register a backend in the local norns server */
int 
norns_register_backend(struct norns_cred* auth, norns_backend_t* backend) {

    if(auth == NULL || !validate_backend(backend)) {
        return NORNS_EBADARGS;
    }

    const char* const nsid = backend->b_nsid;

    return send_backend_request(NORNS_BACKEND_REGISTER, auth, nsid, backend);
}

/* Unregister a backend from the local norns server */
int 
norns_unregister_backend(struct norns_cred* auth, const char* nsid) {

    if(auth == NULL || nsid == NULL) {
        return NORNS_EBADARGS;
    }

    return send_backend_request(NORNS_BACKEND_UNREGISTER, auth, nsid, NULL);
}

norns_backend_t 
NORNS_BACKEND(const char* nsid, norns_flags_t flags, 
              const char* mount_point, uint32_t quota) {

    norns_backend_t b;

    norns_backend_init(&b, nsid, flags, mount_point, quota);

    return b;
}

void 
norns_backend_init(norns_backend_t* backend, const char* nsid, 
                   norns_flags_t flags, const char* mount_point,
                   uint32_t quota) {

    if(backend == NULL) {
        return;
    }

    // mount_point can be NULL for certain backends
    if(nsid == NULL) {
        memset(backend, 0, sizeof(*backend));
        return;
    }

    backend->b_nsid = nsid;
    backend->b_type = flags;
    backend->b_mount = mount_point;
    backend->b_quota = quota;
}

static bool
validate_backend(norns_backend_t* backend) {

    if(backend == NULL) {
        return false;
    }

    if(backend->b_type != NORNS_BACKEND_PROCESS_MEMORY) {
        return (backend->b_nsid != NULL) &&
            (strncmp(backend->b_nsid, "", 1) != 0) &&
            (backend->b_mount != NULL) &&
            (strncmp(backend->b_mount, "", 1) != 0) &&
            (backend->b_quota > 0);
    }

    return (backend->b_nsid != NULL);
}


static bool
validate_job(norns_job_t* job) {

    return (job != NULL) && 
           (job->j_hosts != NULL) && 
           (job->j_nhosts) != 0 &&
           (job->j_backends != NULL) && 
           (job->j_nbackends != 0);
}

norns_job_t 
NORNS_JOB(const char** hosts, size_t nhosts, 
                      norns_backend_t** backends, size_t nbackends) {

    norns_job_t job;
    norns_job_init(&job, hosts, nhosts, backends, nbackends);
    return job;
}

void 
norns_job_init(norns_job_t* job, const char** hosts, size_t nhosts, 
                    norns_backend_t** backends, size_t nbackends) {

    if(job == NULL) {
        return;
    }

    if(hosts == NULL || nhosts == 0 || backends == NULL || nbackends == 0) {
        memset(job, 0, sizeof(*job));
        return;
    }

    job->j_hosts = hosts;
    job->j_nhosts = nhosts;
    job->j_backends = backends;
    job->j_nbackends = nbackends;
}
