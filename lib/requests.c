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

#include <stdio.h>

#include <norns.h>
#include "messages.pb-c.h"
#include "xmalloc.h"
#include "xstring.h"

#include "requests.h"

void 
job_message_free(Norns__Rpc__Request__Job* msg) {

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

int 
job_message_init(struct norns_job* job, Norns__Rpc__Request__Job** msg) {

    int rv;
    *msg = NULL;

    assert(job != NULL);

    Norns__Rpc__Request__Job* jobmsg = 
        (Norns__Rpc__Request__Job*) xmalloc(sizeof(*jobmsg));

    if(jobmsg == NULL) {
        rv = NORNS_ENOMEM;
        goto error_cleanup;
    }

    norns__rpc__request__job__init(jobmsg);

    // add hosts
    jobmsg->n_hosts = job->jb_nhosts; // save number of repeated hosts
    jobmsg->hosts = xmalloc(jobmsg->n_hosts*sizeof(char*));

    if(jobmsg->hosts == NULL) {
        rv = NORNS_ENOMEM;
        goto error_cleanup;
    }

    for(size_t i=0; i<job->jb_nhosts; ++i){
        if(job->jb_hosts[i] == NULL){
            continue;
        }

        jobmsg->hosts[i] = xstrdup(job->jb_hosts[i]);

        if(jobmsg->hosts[i] == NULL) {
            rv = NORNS_ENOMEM;
            goto error_cleanup;
        }
    }

    // add backends
    jobmsg->n_backends = job->jb_nbackends;
    jobmsg->backends = 
        xmalloc(job->jb_nbackends*sizeof(Norns__Rpc__Request__Job__Backend*));
    
    if(jobmsg->backends == NULL){
        rv = NORNS_ENOMEM;
        goto error_cleanup;
    }

    for(size_t i=0; i<job->jb_nbackends; ++i) {
        jobmsg->backends[i] = xmalloc(sizeof(Norns__Rpc__Request__Job__Backend));

        if(jobmsg->backends[i] == NULL) {
            rv = NORNS_ENOMEM;
            goto error_cleanup;
        }

        norns__rpc__request__job__backend__init(jobmsg->backends[i]);
        jobmsg->backends[i]->type = job->jb_backends[i]->b_type;
        jobmsg->backends[i]->mount = xstrdup(job->jb_backends[i]->b_mount);

        if(jobmsg->backends[i]->mount == NULL) {
            rv = NORNS_ENOMEM;
            goto error_cleanup;
        }

        jobmsg->backends[i]->quota = job->jb_backends[i]->b_quota;
    }

    *msg = jobmsg;
    return NORNS_SUCCESS;

error_cleanup:
    if(jobmsg != NULL) {
        job_message_free(jobmsg);
    }
    return rv;
}


static void*
build_buffer_message(struct norns_data_in* src) {

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
build_path_message(void* src, bool is_input_data) {

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

int 
build_task_message(struct norns_iotd* iotdp, Norns__Rpc__Request__Task** msg) {

    int rv;
    *msg = NULL;

    Norns__Rpc__Request__Task* taskmsg =
        (Norns__Rpc__Request__Task*) xmalloc(sizeof(*taskmsg));

    if(taskmsg == NULL) {
        rv = NORNS_ENOMEM;
        goto cleanup;
    }

    norns__rpc__request__task__init(taskmsg);

    taskmsg->optype = iotdp->io_optype;

    // construct source
    taskmsg->source = xmalloc(sizeof(Norns__Rpc__Request__Task__DataIn));

    if(taskmsg->source == NULL) {
        rv = NORNS_ENOMEM;
        goto cleanup;
    }

    norns__rpc__request__task__data_in__init(taskmsg->source);

    taskmsg->source->type = iotdp->io_src.in_type;

    if(iotdp->io_src.in_type == NORNS_BACKEND_PROCESS_MEMORY) {
        taskmsg->source->buffer = build_buffer_message(&iotdp->io_src);
        taskmsg->source->path = NULL;

        if(taskmsg->source->buffer == NULL) {
            rv = NORNS_ENOMEM;
            goto cleanup;
        }
    }
    else {
        taskmsg->source->path = build_path_message(&iotdp->io_src, true);
        taskmsg->source->buffer = NULL;

        if(taskmsg->source->path == NULL) {
            rv = NORNS_ENOMEM;
            goto cleanup;
        }
    }

    fprintf(stderr, "destination\n");
    // construct destination
    taskmsg->destination = xmalloc(sizeof(Norns__Rpc__Request__Task__DataOut));

    if(taskmsg->destination == NULL) {
        rv = NORNS_ENOMEM;
        goto cleanup;
    }

    norns__rpc__request__task__data_out__init(taskmsg->destination);

    taskmsg->destination->type = iotdp->io_dst.out_type;

    taskmsg->destination->path = build_path_message(&iotdp->io_dst, false);

    if(taskmsg->destination->path == NULL) {
        rv = NORNS_ENOMEM;
        goto cleanup;
    }

    *msg = taskmsg;
    return NORNS_SUCCESS;


cleanup:
    if(taskmsg != NULL) {
        free_task_message(taskmsg);
    }

    return rv;
}

void free_task_message(Norns__Rpc__Request__Task* msg) {

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
