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

#include <stdarg.h>

#include "norns.h"
#include "nornsctl.h"
#include "messages.pb-c.h"
#include "xmalloc.h"
#include "xstring.h"
#include "log.h"

#include "requests.h"

/* helpers */
static Norns__Rpc__Request__Task* 
build_task_msg(const norns_iotask_t* iotdp);
static void 
free_task_msg(Norns__Rpc__Request__Task* msg);

static Norns__Rpc__Request__Namespace* 
build_namespace_message(const char* nsid, const nornsctl_backend_t* backend);
static void 
free_namespace_message(Norns__Rpc__Request__Namespace* msg);

static Norns__Rpc__Request__Namespace__Backend* 
build_backend_message(const nornsctl_backend_t* backend);
static void 
free_backend_message(Norns__Rpc__Request__Namespace__Backend* msg);

static Norns__Rpc__Request__JobLimits* 
build_job_limits_msg(const nornsctl_job_limit_t* lim);
static void 
free_job_limits_msg(Norns__Rpc__Request__JobLimits* msg);

static Norns__Rpc__Request__Job* 
build_job_msg(const nornsctl_job_t* job);
static void 
free_job_msg(Norns__Rpc__Request__Job* msg);

static Norns__Rpc__Request__Task__Resource* 
build_resource_msg(const norns_resource_t* res);
static void 
free_resource_msg(Norns__Rpc__Request__Task__Resource* msg);

static Norns__Rpc__Request__Task__MemoryRegion* 
build_membuf_msg(const norns_memory_region_t* buf);
static void 
free_membuf_msg(Norns__Rpc__Request__Task__MemoryRegion* msg);

static Norns__Rpc__Request__Task__PosixPath* 
build_path_msg(const norns_posix_path_t* path);
static void 
free_path_msg(Norns__Rpc__Request__Task__PosixPath* msg);

static Norns__Rpc__Request__Command* 
build_command_msg(const nornsctl_command_t cmd, const void* args);
static void
free_command_msg(Norns__Rpc__Request__Command* msg) __attribute__((unused));

static int 
encode_request_type(norns_msgtype_t type);
static norns_msgtype_t 
decode_response_type(int type);

static int 
encode_request_type(norns_msgtype_t type) {
    switch(type) {
        case NORNS_IOTASK_SUBMIT:
            return NORNS__RPC__REQUEST__TYPE__IOTASK_SUBMIT;
        case NORNS_IOTASK_STATUS:
            return NORNS__RPC__REQUEST__TYPE__IOTASK_STATUS;
        case NORNS_PING:
            return NORNS__RPC__REQUEST__TYPE__PING;
        case NORNS_JOB_REGISTER:
            return NORNS__RPC__REQUEST__TYPE__JOB_REGISTER;
        case NORNS_JOB_UPDATE:
            return NORNS__RPC__REQUEST__TYPE__JOB_UPDATE;
        case NORNS_JOB_UNREGISTER:
            return NORNS__RPC__REQUEST__TYPE__JOB_UNREGISTER;
        case NORNS_PROCESS_ADD:
            return NORNS__RPC__REQUEST__TYPE__PROCESS_ADD;
        case NORNS_PROCESS_REMOVE:
            return NORNS__RPC__REQUEST__TYPE__PROCESS_REMOVE;
        case NORNS_NAMESPACE_REGISTER:
            return NORNS__RPC__REQUEST__TYPE__NAMESPACE_REGISTER;
        case NORNS_NAMESPACE_UPDATE:
            return NORNS__RPC__REQUEST__TYPE__NAMESPACE_UPDATE;
        case NORNS_NAMESPACE_UNREGISTER:
            return NORNS__RPC__REQUEST__TYPE__NAMESPACE_UNREGISTER;
        case NORNSCTL_GLOBAL_STATUS:
            return NORNS__RPC__REQUEST__TYPE__GLOBAL_STATUS;
        case NORNSCTL_COMMAND:
            return NORNS__RPC__REQUEST__TYPE__CTL_COMMAND;
        default:
            return -1;
    }
}

static norns_msgtype_t 
decode_response_type(int norns_rpc_type) {
    switch(norns_rpc_type) {
        case NORNS__RPC__RESPONSE__TYPE__IOTASK_SUBMIT:
            return NORNS_IOTASK_SUBMIT;
        case NORNS__RPC__RESPONSE__TYPE__IOTASK_STATUS:
            return NORNS_IOTASK_STATUS;
        case NORNS__RPC__RESPONSE__TYPE__PING:
            return NORNS_PING;
        case NORNS__RPC__RESPONSE__TYPE__JOB_REGISTER:
            return NORNS_JOB_REGISTER;
        case NORNS__RPC__RESPONSE__TYPE__JOB_UPDATE:
            return NORNS_JOB_UPDATE;
        case NORNS__RPC__RESPONSE__TYPE__JOB_UNREGISTER:
            return NORNS_JOB_UNREGISTER;
        case NORNS__RPC__RESPONSE__TYPE__PROCESS_ADD:
            return NORNS_PROCESS_ADD;
        case NORNS__RPC__RESPONSE__TYPE__PROCESS_REMOVE:
            return NORNS_PROCESS_REMOVE;
        case NORNS__RPC__RESPONSE__TYPE__NAMESPACE_REGISTER:
            return NORNS_NAMESPACE_REGISTER;
        case NORNS__RPC__RESPONSE__TYPE__NAMESPACE_UPDATE:
            return NORNS_NAMESPACE_UPDATE;
        case NORNS__RPC__RESPONSE__TYPE__NAMESPACE_UNREGISTER:
            return NORNS_NAMESPACE_UNREGISTER;
        case NORNS__RPC__RESPONSE__TYPE__GLOBAL_STATUS:
            return NORNSCTL_GLOBAL_STATUS;
        case NORNS__RPC__REQUEST__TYPE__CTL_COMMAND:
            return NORNSCTL_COMMAND;
        case NORNS__RPC__RESPONSE__TYPE__BAD_REQUEST:
            // intentionally fall through
        default:
            return NORNS_BAD_RPC;
    }
}

Norns__Rpc__Request*
build_request_msg(norns_msgtype_t type, va_list ap) {

    Norns__Rpc__Request* req_msg = NULL;

    if((req_msg = (Norns__Rpc__Request*) xmalloc(sizeof(*req_msg))) == NULL) {
        goto cleanup_on_error;
    }

    norns__rpc__request__init(req_msg);

    if( (int) (req_msg->type = encode_request_type(type)) < 0) {
        ERR("Encoding of request type failed");
        goto cleanup_on_error;
    }

    switch(type) {

        case NORNSCTL_COMMAND:
        {
            const nornsctl_command_t cmd = va_arg(ap, nornsctl_command_t);
            const void* args = va_arg(ap, void*);

            if((req_msg->command = build_command_msg(cmd, args)) == NULL) {
                goto cleanup_on_error;
            }

            break;
        }

        case NORNS_IOTASK_SUBMIT:
        case NORNS_IOTASK_STATUS:
        {
            const norns_iotask_t* task = va_arg(ap, norns_iotask_t*);

            if((req_msg->task = build_task_msg(task)) == NULL) {
                goto cleanup_on_error;
            }

            break;
        }

        case NORNSCTL_GLOBAL_STATUS:
        case NORNS_PING:
        {
            break;
        }

        case NORNS_JOB_REGISTER:
        case NORNS_JOB_UPDATE:
        case NORNS_JOB_UNREGISTER:
        {
            const uint32_t jobid = va_arg(ap, uint32_t);
            const nornsctl_job_t* job = va_arg(ap, nornsctl_job_t*);

            req_msg->has_jobid = true;
            req_msg->jobid = jobid;

            if(type == NORNS_JOB_UNREGISTER) {
                req_msg->job = NULL;
            }
            else {
                if((req_msg->job = build_job_msg(job)) == NULL) {
                    goto cleanup_on_error;
                }
            }

            break;
        }

        case NORNS_PROCESS_ADD:
        case NORNS_PROCESS_REMOVE:
        {
            const uint32_t jobid = va_arg(ap, uint32_t);
            const uid_t uid = va_arg(ap, uid_t);
            const gid_t gid = va_arg(ap, gid_t);
            const pid_t pid = va_arg(ap, pid_t);

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

        case NORNS_NAMESPACE_REGISTER:
        case NORNS_NAMESPACE_UPDATE:
        case NORNS_NAMESPACE_UNREGISTER:
        {
            const char* const nsid =
                va_arg(ap, const char* const);
            const nornsctl_backend_t* backend = 
                va_arg(ap, const nornsctl_backend_t*);

            if((req_msg->nspace = 
                    build_namespace_message(nsid, backend)) == NULL) {
                goto cleanup_on_error;
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
free_backend_message(Norns__Rpc__Request__Namespace__Backend* msg) {
    //TODO
    (void) msg;
}

static void 
free_job_limits_msg(Norns__Rpc__Request__JobLimits* msg) {
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

    if(msg->limits != NULL) {

        for(size_t i=0; i<msg->n_limits; ++i) {
            if(msg->limits[i] != NULL) {
                if(msg->limits[i]->nsid != NULL) {
                    xfree(msg->limits[i]->nsid);
                }
                xfree(msg->limits[i]);
            }
        }
        xfree(msg->limits);
    }
    xfree(msg);
}

static Norns__Rpc__Request__Namespace* 
build_namespace_message(const char* nsid, const nornsctl_backend_t* backend) {

    assert(nsid != NULL);

    Norns__Rpc__Request__Namespace* nsmsg =
        (Norns__Rpc__Request__Namespace*) xmalloc(sizeof(*nsmsg));

    if(nsmsg == NULL) {
        ERR("!xmalloc");
        return NULL;
    }

    norns__rpc__request__namespace__init(nsmsg);

    nsmsg->nsid = xstrdup(nsid);

    if(nsmsg->nsid == NULL) {
        ERR("!xstrdup");
        goto error_cleanup;
    }

    if(backend != NULL) {
        if((nsmsg->backend = build_backend_message(backend)) == NULL) {
            goto error_cleanup;
        }
    }
    else {
        // request must be NORNS_NAMESPACE_UNREGISTER
        nsmsg->backend = NULL;
    }

    return nsmsg;

error_cleanup:
    if(nsmsg != NULL) {
        free_namespace_message(nsmsg);
    }

    return NULL;
}

static void 
free_namespace_message(Norns__Rpc__Request__Namespace* msg) {
    (void) msg;
}

static Norns__Rpc__Request__Namespace__Backend*
build_backend_message(const nornsctl_backend_t* backend) {

    assert(backend != NULL);

    Norns__Rpc__Request__Namespace__Backend* backendmsg = 
        (Norns__Rpc__Request__Namespace__Backend*) xmalloc(sizeof(*backendmsg));

    if(backendmsg == NULL) {
        return NULL;
    }

    norns__rpc__request__namespace__backend__init(backendmsg);

    backendmsg->type = backend->b_type;

    // n_mount might be NULL for some namespaces 
    // (e.g. NORNS_NS_PROCESS_MEMORY)
    if(backend->b_mount != NULL) {
        backendmsg->mount = xstrdup(backend->b_mount);

        if(backendmsg->mount == NULL) {
            goto error_cleanup;
        }
    }
    else {
        backendmsg->mount = NULL;
    }

    backendmsg->capacity = backend->b_capacity;

    return backendmsg;

error_cleanup:

    if(backendmsg != NULL) {
        free_backend_message(backendmsg);
    }

    return NULL;
}

static Norns__Rpc__Request__JobLimits* 
build_job_limits_msg(const nornsctl_job_limit_t* lim) {

    assert(lim != NULL);

    Norns__Rpc__Request__JobLimits* limmsg = 
        (Norns__Rpc__Request__JobLimits*) xmalloc(sizeof(*limmsg));

    if(limmsg == NULL) {
        return NULL;
    }

    norns__rpc__request__job_limits__init(limmsg);

    limmsg->nsid = xstrdup(lim->l_nsid);

    if(limmsg->nsid == NULL) {
        goto error_cleanup;
    }

    limmsg->quota = lim->l_quota;

    return limmsg;

error_cleanup:

    if(limmsg != NULL) {
        free_job_limits_msg(limmsg);
    }

    return NULL;
}


static Norns__Rpc__Request__Job*
build_job_msg(const nornsctl_job_t* job) {

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

    // add limits
    jobmsg->n_limits = job->j_nlimits;
    jobmsg->limits = 
        xmalloc(job->j_nlimits*sizeof(Norns__Rpc__Request__Namespace__Backend*));
    
    if(jobmsg->limits == NULL){
        goto error_cleanup;
    }

    for(size_t i=0; i<job->j_nlimits; ++i) {

        jobmsg->limits[i] = build_job_limits_msg(job->j_limits[i]);

        if(jobmsg->limits[i] == NULL) {
            goto error_cleanup;
        }
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

    const char* hostname = NULL;
    const char* datapath = NULL;
    Norns__Rpc__Request__Task__PosixPath* msg = xmalloc(sizeof(*msg));

    if(msg == NULL) {
        return NULL;
    }

    norns__rpc__request__task__posix_path__init(msg);

    msg->nsid = xstrdup(path->p_nsid);

    if(msg->nsid == NULL) {
        goto oom_cleanup;
    }


    hostname = path->p_host;
    datapath = path->p_path;

    if(hostname != NULL) {
        msg->hostname = xstrdup(hostname);

        if(msg->hostname == NULL) {
            goto oom_cleanup;
        }
    }

    if(datapath != NULL) {
        msg->datapath = xstrdup(datapath);

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

    taskmsg->taskid = task->t_id;
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

Norns__Rpc__Request__Command* 
build_command_msg(const nornsctl_command_t cmd, const void* args) {

    (void) args;

    Norns__Rpc__Request__Command* msg = xmalloc(sizeof(*msg)); 

    if(msg == NULL) {
        return NULL;
    }

    norns__rpc__request__command__init(msg);
    msg->id = cmd;

    return msg;
}

void
free_command_msg(Norns__Rpc__Request__Command* msg) {
    // TODO
    (void) msg;
}

int
pack_to_buffer(norns_msgtype_t type, norns_msgbuffer_t* buf, ...) {

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
unpack_from_buffer(norns_msgbuffer_t* buf, norns_response_t* response) {

    Norns__Rpc__Response* rpc_resp = NULL;
    void* resp_buf = buf->b_data;
    size_t resp_len = buf->b_size;

    rpc_resp = norns__rpc__response__unpack(NULL, resp_len, resp_buf);

    if(rpc_resp == NULL) {
        return NORNS_ERPCRECVFAILED;
    }

    response->r_type = decode_response_type(rpc_resp->type);
    response->r_error_code = rpc_resp->error_code;

    switch(response->r_type) {
        case NORNS_IOTASK_SUBMIT:
            if(!rpc_resp->has_taskid) {
                return NORNS_ERPCRECVFAILED;
            }
            response->r_taskid = rpc_resp->taskid;
            break;

        case NORNS_IOTASK_STATUS:
            if(rpc_resp->stats == NULL) {
                return NORNS_ERPCRECVFAILED;
            }
            response->r_status = rpc_resp->stats->status;
            response->r_task_error = rpc_resp->stats->task_error;
            response->r_errno = rpc_resp->stats->sys_errnum;
            break;

        case NORNSCTL_GLOBAL_STATUS:
            if(rpc_resp->gstats == NULL) {
                return NORNS_ERPCRECVFAILED;
            }
            response->r_running_tasks = rpc_resp->gstats->running_tasks;
            response->r_pending_tasks = rpc_resp->gstats->pending_tasks;
            response->r_eta = rpc_resp->gstats->eta;
            break;

        default:
            break;
    }

    return NORNS_SUCCESS;
}

