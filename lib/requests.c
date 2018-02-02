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

static Norns__Rpc__Request__Task* build_task_msg(const struct norns_iotd* iotdp);
static void free_task_msg(Norns__Rpc__Request__Task* msg);
static Norns__Rpc__Request__Backend* build_backend_msg(const struct norns_backend* backend);
static void free_backend_msg(Norns__Rpc__Request__Backend* msg);
static Norns__Rpc__Request__Job* build_job_msg(const struct norns_job* job);
static void free_job_msg(Norns__Rpc__Request__Job* msg);
static void* build_membuf_msg(const struct norns_data_in* src);
static int remap_request(norns_rpc_type_t type);
static norns_rpc_type_t remap_response(int type);

int 
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

norns_rpc_type_t 
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
            const struct norns_iotd* iotdp = va_arg(ap, struct norns_iotd*);

            if((req_msg->type = remap_request(type)) < 0) {
                goto cleanup_on_error;
            }

            if((req_msg->task = build_task_msg(iotdp)) == NULL) {
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
            const struct norns_job* job = va_arg(ap, struct norns_job*);

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
            const char* const prefix =
                va_arg(ap, const char* const);
            const struct norns_backend* backend = 
                va_arg(ap, const struct norns_backend*);

            (void) auth;

            if((req_msg->type = remap_request(type)) < 0) {
                goto cleanup_on_error;
            }

            if(type == NORNS_UNREGISTER_BACKEND) {
                req_msg->prefix = xstrdup(prefix);

                if(req_msg->prefix == NULL) {
                    goto cleanup_on_error;
                }

                req_msg->backend = NULL;
            }
            else {

                req_msg->prefix = NULL;

                if((req_msg->backend = build_backend_msg(backend)) == NULL) {
                    goto cleanup_on_error;
                }
            }

            break;
        }

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
}

void
free_backend_msg(Norns__Rpc__Request__Backend* msg) {
    //TODO
}

void 
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

Norns__Rpc__Request__Backend*
build_backend_msg(const struct norns_backend* backend) {

    assert(backend != NULL);

    Norns__Rpc__Request__Backend* backendmsg = 
        (Norns__Rpc__Request__Backend*) xmalloc(sizeof(*backendmsg));

    if(backendmsg == NULL) {
        return NULL;
    }

    norns__rpc__request__backend__init(backendmsg);

    backendmsg->prefix = xstrdup(backend->b_prefix);

    if(backendmsg->prefix == NULL) {
        goto error_cleanup;
    }

    backendmsg->type = backend->b_type;
    backendmsg->mount = xstrdup(backend->b_mount);

    if(backendmsg->mount == NULL) {
        goto error_cleanup;
    }

    backendmsg->quota = backend->b_quota;

    return backendmsg;

error_cleanup:

    if(backendmsg != NULL) {
        free_backend_msg(backendmsg);
    }

    return NULL;
}

Norns__Rpc__Request__Job*
build_job_msg(const struct norns_job* job) {

    assert(job != NULL);

    Norns__Rpc__Request__Job* jobmsg = 
        (Norns__Rpc__Request__Job*) xmalloc(sizeof(*jobmsg));

    if(jobmsg == NULL) {
        return NULL;
    }

    norns__rpc__request__job__init(jobmsg);

    // add hosts
    jobmsg->n_hosts = job->jb_nhosts; // save number of repeated hosts
    jobmsg->hosts = xmalloc(jobmsg->n_hosts*sizeof(char*));

    if(jobmsg->hosts == NULL) {
        goto error_cleanup;
    }

    for(size_t i=0; i<job->jb_nhosts; ++i){
        if(job->jb_hosts[i] == NULL){
            continue;
        }

        jobmsg->hosts[i] = xstrdup(job->jb_hosts[i]);

        if(jobmsg->hosts[i] == NULL) {
            goto error_cleanup;
        }
    }

    // add backends
    jobmsg->n_backends = job->jb_nbackends;
    jobmsg->backends = 
        xmalloc(job->jb_nbackends*sizeof(Norns__Rpc__Request__Backend*));
    
    if(jobmsg->backends == NULL){
        goto error_cleanup;
    }

    for(size_t i=0; i<job->jb_nbackends; ++i) {

        jobmsg->backends[i] = build_backend_msg(job->jb_backends[i]);

        if(jobmsg->backends[i] == NULL) {
            goto error_cleanup;
        }

        /*
        jobmsg->backends[i] = xmalloc(sizeof(Norns__Rpc__Request__Backend));

        if(jobmsg->backends[i] == NULL) {
            goto error_cleanup;
        }

        norns__rpc__request__backend__init(jobmsg->backends[i]);
        jobmsg->backends[i]->type = job->jb_backends[i]->b_type;
        jobmsg->backends[i]->mount = xstrdup(job->jb_backends[i]->b_mount);

        if(jobmsg->backends[i]->mount == NULL) {
            goto error_cleanup;
        }

        jobmsg->backends[i]->quota = job->jb_backends[i]->b_quota;
        */
    }

    return jobmsg;

error_cleanup:
    if(jobmsg != NULL) {
        free_job_msg(jobmsg);
    }
    return NULL;
}


void*
build_membuf_msg(const struct norns_data_in* src) {

    Norns__Rpc__Request__Task__MemoryBuffer* msg = 
        xmalloc(sizeof(*msg));

    if(msg == NULL) {
        return NULL;
    }

    msg->address = (uint64_t) src->in_buffer.b_addr;
    msg->size = src->in_buffer.b_size;

    return msg;
}

static void*
build_path_msg(const void* src, bool is_input_data) {

    const char* src_hostname = NULL;
    const char* src_datapath = NULL;
    Norns__Rpc__Request__Task__FSPath* msg = xmalloc(sizeof(*msg));

    if(msg == NULL) {
        return NULL;
    }

    norns__rpc__request__task__fspath__init(msg);

    if(is_input_data) {
        src_hostname = ((struct norns_data_in*) src)->in_path.p_hostname;
        src_datapath = ((struct norns_data_in*) src)->in_path.p_datapath;
    }
    else {
        src_hostname = ((struct norns_data_out*) src)->out_path.p_hostname;
        src_datapath = ((struct norns_data_out*) src)->out_path.p_datapath;
    }

    if(src_hostname != NULL) {
        msg->hostname = xstrdup(src_hostname);

        if(msg->hostname == NULL) {
            goto cleanup;
        }
    }

    if(src_datapath != NULL) {
        msg->datapath = xstrdup(src_datapath);

        if(msg->datapath == NULL) {
            goto cleanup;
        }
    }

    return msg;

cleanup:
    if(msg->hostname != NULL) { 
        xfree(msg->hostname);
    }

    if(msg->datapath == NULL) {
        xfree(msg->datapath);
    }

    if(msg != NULL) {
        xfree(msg);
    }

    return NULL;
}

Norns__Rpc__Request__Task*
build_task_msg(const struct norns_iotd* iotdp) {

    Norns__Rpc__Request__Task* taskmsg =
        (Norns__Rpc__Request__Task*) xmalloc(sizeof(*taskmsg));

    if(taskmsg == NULL) {
        return NULL;
    }

    norns__rpc__request__task__init(taskmsg);

    taskmsg->optype = iotdp->io_optype;

    // construct source
    taskmsg->source = xmalloc(sizeof(Norns__Rpc__Request__Task__DataIn));

    if(taskmsg->source == NULL) {
        goto cleanup_on_error;
    }

    norns__rpc__request__task__data_in__init(taskmsg->source);

    taskmsg->source->type = iotdp->io_src.in_type;

    if(iotdp->io_src.in_type == NORNS_BACKEND_PROCESS_MEMORY) {
        taskmsg->source->buffer = build_membuf_msg(&iotdp->io_src);
        taskmsg->source->path = NULL;

        if(taskmsg->source->buffer == NULL) {
            goto cleanup_on_error;
        }
    }
    else {
        taskmsg->source->path = build_path_msg(&iotdp->io_src, true);
        taskmsg->source->buffer = NULL;

        if(taskmsg->source->path == NULL) {
            goto cleanup_on_error;
        }
    }

    // construct destination
    taskmsg->destination = xmalloc(sizeof(Norns__Rpc__Request__Task__DataOut));

    if(taskmsg->destination == NULL) {
        goto cleanup_on_error;
    }

    norns__rpc__request__task__data_out__init(taskmsg->destination);

    taskmsg->destination->type = iotdp->io_dst.out_type;

    taskmsg->destination->path = build_path_msg(&iotdp->io_dst, false);

    if(taskmsg->destination->path == NULL) {
        goto cleanup_on_error;
    }

    return taskmsg;

cleanup_on_error:
    if(taskmsg != NULL) {
        free_task_msg(taskmsg);
    }

    return NULL;
}

void free_task_msg(Norns__Rpc__Request__Task* msg) {

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

