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

#include <stdarg.h>

#include <norns.h>
#include "messages.pb-c.h"
#include "xmalloc.h"
#include "xstring.h"

#include "requests.h"

/* helpers */
static Norns__Rpc__Request__Task* build_task_msg(const norns_iotask_t* iotdp);
static void free_task_msg(Norns__Rpc__Request__Task* msg);
static Norns__Rpc__Request__Backend* build_backend_msg(const norns_backend_t* backend);
static void free_backend_msg(Norns__Rpc__Request__Backend* msg);
static Norns__Rpc__Request__Job* build_job_msg(const norns_job_t* job);
static void free_job_msg(Norns__Rpc__Request__Job* msg);

static Norns__Rpc__Request__Task__Resource* build_resource_msg(const norns_resource_t* res);
static void free_resource_msg(Norns__Rpc__Request__Task__Resource* msg);

static Norns__Rpc__Request__Task__MemoryRegion* build_membuf_msg(const norns_memory_region_t* buf);
static void free_membuf_msg(Norns__Rpc__Request__Task__MemoryRegion* msg);

static Norns__Rpc__Request__Task__PosixPath* build_path_msg(const norns_posix_path_t* path);
static void free_path_msg(Norns__Rpc__Request__Task__PosixPath* msg);


static int remap_request(norns_rpc_type_t type);
static norns_rpc_type_t remap_response(int type);

static int 
remap_request(norns_rpc_type_t type) {
    switch(type) {
        case NORNS_SUBMIT_IOTASK:
            return NORNS__RPC__REQUEST__TYPE__SUBMIT_IOTASK;
        case NORNS_PING:
            return NORNS__RPC__REQUEST__TYPE__PING;
        case NORNS_REGISTER_JOB:
            return NORNS__RPC__REQUEST__TYPE__REGISTER_JOB;
        case NORNS_UPDATE_JOB:
            return NORNS__RPC__REQUEST__TYPE__UPDATE_JOB;
        case NORNS_UNREGISTER_JOB:
            return NORNS__RPC__REQUEST__TYPE__UNREGISTER_JOB;
        case NORNS_ADD_PROCESS:
            return NORNS__RPC__REQUEST__TYPE__ADD_PROCESS;
        case NORNS_REMOVE_PROCESS:
            return NORNS__RPC__REQUEST__TYPE__REMOVE_PROCESS;
        case NORNS_REGISTER_BACKEND:
            return NORNS__RPC__REQUEST__TYPE__REGISTER_BACKEND;
        case NORNS_UPDATE_BACKEND:
            return NORNS__RPC__REQUEST__TYPE__UPDATE_BACKEND;
        case NORNS_UNREGISTER_BACKEND:
            return NORNS__RPC__REQUEST__TYPE__UNREGISTER_BACKEND;
        default:
            return -1;
    }
}

static norns_rpc_type_t 
remap_response(int norns_rpc_type) {
    switch(norns_rpc_type) {
        case NORNS__RPC__RESPONSE__TYPE__SUBMIT_IOTASK:
            return NORNS_SUBMIT_IOTASK;
        case NORNS__RPC__RESPONSE__TYPE__PING:
            return NORNS_PING;
        case NORNS__RPC__RESPONSE__TYPE__REGISTER_JOB:
            return NORNS_REGISTER_JOB;
        case NORNS__RPC__RESPONSE__TYPE__UPDATE_JOB:
            return NORNS_UPDATE_JOB;
        case NORNS__RPC__RESPONSE__TYPE__UNREGISTER_JOB:
            return NORNS_UNREGISTER_JOB;
        case NORNS__RPC__RESPONSE__TYPE__ADD_PROCESS:
            return NORNS_ADD_PROCESS;
        case NORNS__RPC__RESPONSE__TYPE__REMOVE_PROCESS:
            return NORNS_REMOVE_PROCESS;
        case NORNS__RPC__REQUEST__TYPE__REGISTER_BACKEND:
            return NORNS_REGISTER_BACKEND;
        case NORNS__RPC__REQUEST__TYPE__UPDATE_BACKEND:
            return NORNS_UPDATE_BACKEND;
        case NORNS__RPC__REQUEST__TYPE__UNREGISTER_BACKEND:
            return NORNS_UNREGISTER_BACKEND;
        case NORNS__RPC__RESPONSE__TYPE__BAD_REQUEST:
            // intentionally fall through
        default:
            return NORNS_BAD_RPC;
    }
}

Norns__Rpc__Request*
build_request_msg(norns_rpc_type_t type, va_list ap) {

    Norns__Rpc__Request* req_msg = NULL;

    if((req_msg = (Norns__Rpc__Request*) xmalloc(sizeof(*req_msg))) == NULL) {
        goto cleanup_on_error;
    }

    norns__rpc__request__init(req_msg);

    switch(type) {
        case NORNS_SUBMIT_IOTASK:
        {
            const norns_iotask_t* task = va_arg(ap, norns_iotask_t*);

            if((req_msg->type = remap_request(type)) < 0) {
                goto cleanup_on_error;
            }

            if((req_msg->task = build_task_msg(task)) == NULL) {
                goto cleanup_on_error;
            }

            break;
        }

        case NORNS_PING:
        {

            if((req_msg->type = remap_request(type)) < 0) {
                goto cleanup_on_error;
            }
            break;
        }

        case NORNS_REGISTER_JOB:
        case NORNS_UPDATE_JOB:
        case NORNS_UNREGISTER_JOB:
        {
            const struct norns_cred* auth = va_arg(ap, struct norns_cred*);
            const uint32_t jobid = va_arg(ap, uint32_t);
            const norns_job_t* job = va_arg(ap, norns_job_t*);

            (void) auth;

            if((req_msg->type = remap_request(type)) < 0) {
                goto cleanup_on_error;
            }

            req_msg->has_jobid = true;
            req_msg->jobid = jobid;

            if(type == NORNS_UNREGISTER_JOB) {
                req_msg->job = NULL;
            }
            else {
                if((req_msg->job = build_job_msg(job)) == NULL) {
                    goto cleanup_on_error;
                }
            }

            break;
        }

        case NORNS_ADD_PROCESS:
        case NORNS_REMOVE_PROCESS:
        {
            const struct norns_cred* auth = va_arg(ap, struct norns_cred*);
            const uint32_t jobid = va_arg(ap, uint32_t);
            const uid_t uid = va_arg(ap, uid_t);
            const gid_t gid = va_arg(ap, gid_t);
            const pid_t pid = va_arg(ap, pid_t);

            (void) auth;

            if((req_msg->type = remap_request(type)) < 0) {
                goto cleanup_on_error;
            }

            req_msg->has_jobid = true;
            req_msg->jobid = jobid;

            if((req_msg->process = xmalloc(sizeof *req_msg->process)) == NULL) {
                goto cleanup_on_error;
            }

            norns__rpc__request__process__init(req_msg->process);
            req_msg->process->uid = uid;
            req_msg->process->gid = gid;
            req_msg->process->pid = pid;

            break;
        }

        case NORNS_REGISTER_BACKEND:
        case NORNS_UPDATE_BACKEND:
        case NORNS_UNREGISTER_BACKEND:
        {
            const struct norns_cred* auth =
                va_arg(ap, const struct norns_cred*);
            const char* const nsid =
                va_arg(ap, const char* const);
            const norns_backend_t* backend = 
                va_arg(ap, const norns_backend_t*);

            (void) auth;

            if((req_msg->type = remap_request(type)) < 0) {
                goto cleanup_on_error;
            }

            if(type == NORNS_UNREGISTER_BACKEND) {
                req_msg->nsid = xstrdup(nsid);

                if(req_msg->nsid == NULL) {
                    goto cleanup_on_error;
                }

                req_msg->backend = NULL;
            }
            else {

                req_msg->nsid = NULL;

                if((req_msg->backend = build_backend_msg(backend)) == NULL) {
                    goto cleanup_on_error;
                }
            }

            break;
        }

        default:
            goto cleanup_on_error;
    }

    return req_msg;

cleanup_on_error:
    if(req_msg != NULL) {
        xfree(req_msg);//TODO
    }

    return NULL;
}

void
free_request_msg(Norns__Rpc__Request* req) {
    (void) req;
}

static void
free_backend_msg(Norns__Rpc__Request__Backend* msg) {
    //TODO
    (void) msg;
}

static void 
free_job_msg(Norns__Rpc__Request__Job* msg) {

    assert(msg != NULL);

    if(msg->hosts != NULL) {
        for(size_t i=0; i<msg->n_hosts; ++i) {
            if(msg->hosts[i] != NULL) {
                xfree(msg->hosts[i]);
            }
        }
        xfree(msg->hosts);
    }

    if(msg->backends != NULL) {

        for(size_t i=0; i<msg->n_backends; ++i) {
            if(msg->backends[i] != NULL) {
                if(msg->backends[i]->mount != NULL) {
                    xfree(msg->backends[i]->mount);
                }
                xfree(msg->backends[i]);
            }
        }
        xfree(msg->backends);
    }
    xfree(msg);
}

static Norns__Rpc__Request__Backend*
build_backend_msg(const norns_backend_t* backend) {

    assert(backend != NULL);

    Norns__Rpc__Request__Backend* backendmsg = 
        (Norns__Rpc__Request__Backend*) xmalloc(sizeof(*backendmsg));

    if(backendmsg == NULL) {
        return NULL;
    }

    norns__rpc__request__backend__init(backendmsg);

    backendmsg->nsid = xstrdup(backend->b_nsid);

    if(backendmsg->nsid == NULL) {
        goto error_cleanup;
    }

    backendmsg->type = backend->b_type;

    // b_mount might be NULL for some backends 
    // (e.g. NORNS_BACKEND_PROCESS_MEMORY)
    if(backend->b_mount != NULL) {
        backendmsg->mount = xstrdup(backend->b_mount);

        if(backendmsg->mount == NULL) {
            goto error_cleanup;
        }
    }
    else {
        backendmsg->mount = NULL;
    }

    backendmsg->quota = backend->b_quota;

    return backendmsg;

error_cleanup:

    if(backendmsg != NULL) {
        free_backend_msg(backendmsg);
    }

    return NULL;
}

static Norns__Rpc__Request__Job*
build_job_msg(const norns_job_t* job) {

    assert(job != NULL);

    Norns__Rpc__Request__Job* jobmsg = 
        (Norns__Rpc__Request__Job*) xmalloc(sizeof(*jobmsg));

    if(jobmsg == NULL) {
        return NULL;
    }

    norns__rpc__request__job__init(jobmsg);

    // add hosts
    jobmsg->n_hosts = job->j_nhosts; // save number of repeated hosts
    jobmsg->hosts = xmalloc(jobmsg->n_hosts*sizeof(char*));

    if(jobmsg->hosts == NULL) {
        goto error_cleanup;
    }

    for(size_t i=0; i<job->j_nhosts; ++i){
        if(job->j_hosts[i] == NULL){
            continue;
        }

        jobmsg->hosts[i] = xstrdup(job->j_hosts[i]);

        if(jobmsg->hosts[i] == NULL) {
            goto error_cleanup;
        }
    }

    // add backends
    jobmsg->n_backends = job->j_nbackends;
    jobmsg->backends = 
        xmalloc(job->j_nbackends*sizeof(Norns__Rpc__Request__Backend*));
    
    if(jobmsg->backends == NULL){
        goto error_cleanup;
    }

    for(size_t i=0; i<job->j_nbackends; ++i) {

        jobmsg->backends[i] = build_backend_msg(job->j_backends[i]);

        if(jobmsg->backends[i] == NULL) {
            goto error_cleanup;
        }

        /*
        jobmsg->backends[i] = xmalloc(sizeof(Norns__Rpc__Request__Backend));

        if(jobmsg->backends[i] == NULL) {
            goto error_cleanup;
        }

        norns__rpc__request__backend__init(jobmsg->backends[i]);
        jobmsg->backends[i]->type = job->j_backends[i]->b_type;
        jobmsg->backends[i]->mount = xstrdup(job->j_backends[i]->b_mount);

        if(jobmsg->backends[i]->mount == NULL) {
            goto error_cleanup;
        }

        jobmsg->backends[i]->quota = job->j_backends[i]->b_quota;
        */
    }

    return jobmsg;

error_cleanup:
    if(jobmsg != NULL) {
        free_job_msg(jobmsg);
    }
    return NULL;
}

static Norns__Rpc__Request__Task__Resource* 
build_resource_msg(const norns_resource_t* res) {

    Norns__Rpc__Request__Task__Resource* msg = 
        xmalloc(sizeof(*msg));

    if(msg == NULL) {
        goto oom_cleanup;
    }

    norns__rpc__request__task__resource__init(msg);

    msg->type = res->r_flags;
    msg->nsid = xstrdup(res->r_nsid);

    if(msg->nsid == NULL) {
        goto oom_cleanup;
    }

    if(res->r_flags & NORNS_PROCESS_MEMORY) {
        msg->buffer = build_membuf_msg(&res->r_buffer);

        if(msg->buffer == NULL) {
            goto oom_cleanup;
        }
    }
    else {
        assert(res->r_flags & NORNS_POSIX_PATH);

        msg->path = build_path_msg(&res->r_posix_path);

        if(msg->path == NULL) {
            goto oom_cleanup;
        }
    }

    return msg;

oom_cleanup:
    if(msg != NULL) {
        free_resource_msg(msg);
    }
    return NULL;
}

static void 
free_resource_msg(Norns__Rpc__Request__Task__Resource* msg) {

    assert(msg != NULL);

    if(msg->buffer != NULL) {
        free_membuf_msg(msg->buffer);
    }

    if(msg->path != NULL) {
        free_path_msg(msg->path);
    }

    xfree(msg);
}

static Norns__Rpc__Request__Task__MemoryRegion*
build_membuf_msg(const norns_memory_region_t* buf) {

    Norns__Rpc__Request__Task__MemoryRegion* msg = 
        xmalloc(sizeof(*msg));

    if(msg == NULL) {
        return NULL;
    }

    norns__rpc__request__task__memory_region__init(msg);

    msg->address = (uint64_t) buf->b_addr;
    msg->size = buf->b_size;

    return msg;
}

static void 
free_membuf_msg(Norns__Rpc__Request__Task__MemoryRegion* msg) {
    (void) msg;
}

static Norns__Rpc__Request__Task__PosixPath*
build_path_msg(const norns_posix_path_t* path) {

    const char* src_hostname = NULL;
    const char* src_datapath = NULL;
    Norns__Rpc__Request__Task__PosixPath* msg = xmalloc(sizeof(*msg));

    if(msg == NULL) {
        return NULL;
    }

    norns__rpc__request__task__posix_path__init(msg);

    src_hostname = path->p_host;
    src_datapath = path->p_path;

    if(src_hostname != NULL) {
        msg->hostname = xstrdup(src_hostname);

        if(msg->hostname == NULL) {
            goto oom_cleanup;
        }
    }

    if(src_datapath != NULL) {
        msg->datapath = xstrdup(src_datapath);

        if(msg->datapath == NULL) {
            goto oom_cleanup;
        }
    }

    return msg;

oom_cleanup:
    if(msg != NULL) {
        free_path_msg(msg);
    }

    return NULL;
}

static void 
free_path_msg(Norns__Rpc__Request__Task__PosixPath* msg) {

    assert(msg != NULL);

    if(msg->hostname != NULL) { 
        xfree(msg->hostname);
    }

    if(msg->datapath == NULL) {
        xfree(msg->datapath);
    }

    xfree(msg);
}

static Norns__Rpc__Request__Task*
build_task_msg(const norns_iotask_t* task) {

    Norns__Rpc__Request__Task* taskmsg =
        (Norns__Rpc__Request__Task*) xmalloc(sizeof(*taskmsg));

    if(taskmsg == NULL) {
        return NULL;
    }

    norns__rpc__request__task__init(taskmsg);

    taskmsg->optype = task->t_op;

    // construct source
    taskmsg->source = build_resource_msg(&task->t_src);

    if(taskmsg->source == NULL) {
        goto cleanup_on_error;
    }

    // construct destination
    taskmsg->destination = build_resource_msg(&task->t_dst);

    if(taskmsg->destination == NULL) {
        goto cleanup_on_error;
    }

    return taskmsg;

cleanup_on_error:
    if(taskmsg != NULL) {
        free_task_msg(taskmsg);
    }

    return NULL;
}

static void 
free_task_msg(Norns__Rpc__Request__Task* msg) {

    assert(msg != NULL);

    if(msg->source != NULL) {
        if(msg->source->buffer != NULL) {
            xfree(msg->source->buffer);
        }

        if(msg->source->path != NULL) {
            if(msg->source->path->hostname != NULL) {
                xfree(msg->source->path->hostname);
            }

            if(msg->source->path->datapath != NULL) {
                xfree(msg->source->path->datapath);
            }

            xfree(msg->source->path);
        }

        xfree(msg->source);
    }

    if(msg->destination != NULL) {
        xfree(msg->destination);
    }

    xfree(msg);
}

int
pack_to_buffer(norns_rpc_type_t type, msgbuffer_t* buf, ...) {

    Norns__Rpc__Request* req_msg = NULL;
    void* req_buf = NULL;
    size_t req_len = 0;
    va_list ap;
    va_start(ap, buf);

    if((req_msg = build_request_msg(type, ap)) == NULL) {
        goto cleanup_on_error;
    }

    req_len = norns__rpc__request__get_packed_size(req_msg);
    req_buf = xmalloc_nz(req_len);

    if(req_buf == NULL) {
        goto cleanup_on_error;
    }

    norns__rpc__request__pack(req_msg, req_buf);

    buf->b_data = req_buf;
    buf->b_size = req_len;

    va_end(ap);
    free_request_msg(req_msg);

    return NORNS_SUCCESS;

cleanup_on_error:
    if(req_msg != NULL) {
        free_request_msg(req_msg);
    }

    if(req_buf != NULL) {
        xfree(req_buf);
    }

    va_end(ap);
    return NORNS_ENOMEM;
}

int
unpack_from_buffer(msgbuffer_t* buf, norns_response_t* response) {

    Norns__Rpc__Response* rpc_resp = NULL;
    void* resp_buf = buf->b_data;
    size_t resp_len = buf->b_size;

    rpc_resp = norns__rpc__response__unpack(NULL, resp_len, resp_buf);

    if(rpc_resp == NULL) {
        return NORNS_ERPCRECVFAILED;
    }

    response->r_type = remap_response(rpc_resp->type);
    response->r_status = rpc_resp->status;

    return NORNS_SUCCESS;
}

