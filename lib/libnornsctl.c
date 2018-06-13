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
#include <stdlib.h>
#include <errno.h>

#include "nornsctl.h"
#include "libnornsctl-context.h"

#include "config-parser.h"
#include "log.h"
#include "xmalloc.h"
#include "xstring.h"
#include "communication.h"
#include "defaults.h"

#define LIBNORNSCTL_LOG_PREFIX "libnornsctl"

static bool validate_job(norns_job_t* job);
static bool validate_namespace(norns_backend_t* backend);

static void
load_config_file(void) {

    const char* config_file = NULL;

    if((config_file = getenv("NORNS_CONFIG_FILE")) == NULL) {
        config_file = norns_default_config_file;
    }

    DBG("Loading configuration file \"%s\"", config_file);

    if(access(config_file, R_OK) == -1) {
        FATAL("!Failed to access norns configuration file");
    }

    struct kvpair valid_opts[] = {
        { "global_socket",  NULL },
    };

    size_t num_valid_opts = sizeof(valid_opts) / sizeof(valid_opts[0]);

    if(parse_config_file(config_file, valid_opts, num_valid_opts) != 0) {
        FATAL("Failed to parse norns configuration file"); 
    }

    struct libnornsctl_context* ctx = libnornsctl_get_context();

    if(ctx == NULL) {
        FATAL("Failed to retrieve library context");
    }
    
    ctx->config_file = xstrdup(config_file);
    ctx->api_socket = xstrdup(valid_opts[0].val);

    xfree(valid_opts[0].val);
}

__attribute__((constructor))
static void
libnornsctl_init(void) {

    FILE* logfp = stderr;

#ifdef __NORNS_DEBUG__
    // parse relevant environment variables
    if(getenv("NORNS_DEBUG_OUTPUT_TO_STDERR") == NULL) {
        logfp = NULL;
    }
#endif

    log_init(LIBNORNSCTL_LOG_PREFIX, logfp);
    libnornsctl_create_context();
    load_config_file();
}

__attribute__((destructor))
static void
libnornsctl_fini(void) {
    libnornsctl_destroy_context();
}

#ifdef __NORNS_DEBUG__
void
libnornsctl_reload_config_file(void) {
    DBG("Reloading configuration file");
    load_config_file();
}
#endif

/* Control API */
norns_error_t
nornsctl_ping() {
    return send_ping_request();
}

/* Register and describe a batch job */
norns_error_t 
nornsctl_register_job(uint32_t jobid, norns_job_t* job) {

    if(!validate_job(job)) {
        return NORNS_EBADARGS;
    }

    return send_job_request(NORNS_JOB_REGISTER, jobid, job);
}

/* Update an existing batch job */
norns_error_t 
nornsctl_update_job(uint32_t jobid, norns_job_t* job) {

    if(!validate_job(job)) {
        return NORNS_EBADARGS;
    }

    return send_job_request(NORNS_JOB_UPDATE, jobid, job);
}


/* Remove a batch job from the system */
norns_error_t 
nornsctl_unregister_job(uint32_t jobid) {
    return send_job_request(NORNS_JOB_UNREGISTER, jobid, NULL);
}


/* Add a process to a registered batch job */
norns_error_t 
nornsctl_add_process(uint32_t jobid, uid_t uid, gid_t gid, pid_t pid) {
    return send_process_request(NORNS_PROCESS_ADD, jobid, uid, gid, pid);
}


/* Remove a process from a registered batch job */
norns_error_t 
nornsctl_remove_process(uint32_t jobid, uid_t uid, gid_t gid, pid_t pid) {
    return send_process_request(NORNS_PROCESS_REMOVE, jobid, uid, gid, pid);
}

/* Register a namespace in the local norns server */
norns_error_t 
nornsctl_register_namespace(const char* nsid, norns_backend_t* backend) {

    if(nsid == NULL || (strncmp(nsid, "", 1) == 0) || 
       !validate_namespace(backend)) {
        return NORNS_EBADARGS;
    }

    return send_namespace_request(NORNS_NAMESPACE_REGISTER, nsid, backend);
}

/* Update a namespace in the local norns server */
norns_error_t 
nornsctl_update_namespace(const char* nsid, norns_backend_t* backend) {

    return NORNS_ENOTSUPPORTED;

    if(nsid == NULL || (strncmp(nsid, "", 1) == 0) || 
       !validate_namespace(backend)) {
        return NORNS_EBADARGS;
    }
}

/* Unregister a namespace from the local norns server */
norns_error_t 
nornsctl_unregister_namespace(const char* nsid) {

    if(nsid == NULL) {
        return NORNS_EBADARGS;
    }

    return send_namespace_request(NORNS_NAMESPACE_UNREGISTER, nsid, NULL);
}

norns_backend_t 
NORNS_BACKEND(norns_flags_t flags, const char* mount_point, 
                uint32_t capacity) {

    norns_backend_t b;

    norns_backend_init(&b, flags, mount_point, capacity);

    return b;
}

void 
norns_backend_init(norns_backend_t* backend, norns_flags_t flags, 
                     const char* mount_point, uint32_t capacity) {

    if(backend == NULL) {
        return;
    }

    backend->b_type = flags;
    backend->b_mount = mount_point;
    backend->b_capacity = capacity;
}

norns_job_limit_t NORNS_JOB_LIMIT(const char* nsid, uint32_t quota) {

    norns_job_limit_t limit;
    norns_job_limit_init(&limit, nsid, quota);
    return limit;
}

void norns_job_limit_init(norns_job_limit_t* limit, const char* nsid, 
                          uint32_t quota) {

    if(limit == NULL) {
        return;
    }

    if(nsid == NULL) {
        memset(limit, 0, sizeof(*limit));
        return;
    }

    limit->l_nsid = nsid;
    limit->l_quota = quota;
}

norns_job_t 
NORNS_JOB(const char** hosts, size_t nhosts, 
          norns_job_limit_t** limits, size_t nlimits) {

    norns_job_t job;
    norns_job_init(&job, hosts, nhosts, limits, nlimits);
    return job;
}

void 
norns_job_init(norns_job_t* job, const char** hosts, size_t nhosts, 
               norns_job_limit_t** limits, size_t nlimits) {

    if(job == NULL) {
        return;
    }

    if(hosts == NULL || nhosts == 0 || limits == NULL || nlimits == 0) {
        memset(job, 0, sizeof(*job));
        return;
    }

    job->j_hosts = hosts;
    job->j_nhosts = nhosts;
    job->j_limits = limits;
    job->j_nlimits = nlimits;
}

static bool
validate_namespace(norns_backend_t* backend) {

    if(backend == NULL) {
        return false;
    }

    return (backend->b_mount != NULL) &&
            (strncmp(backend->b_mount, "", 1) != 0) &&
            (backend->b_capacity > 0);
}


static bool
validate_job(norns_job_t* job) {

    return (job != NULL) && 
           (job->j_hosts != NULL) && 
           (job->j_nhosts) != 0 &&
           (job->j_limits != NULL) && 
           (job->j_nlimits != 0);
}
