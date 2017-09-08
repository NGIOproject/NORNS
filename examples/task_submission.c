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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <string.h>

#include <norns.h>

const char* ex_hosts[5] = {
    "node-00",
    "node-01",
    "node-02",
    "node-03",
    "node-04",
};

int main(int argc, char* argv[]) {

    (void) argc;
    (void) argv;
    struct norns_cred cred;

    // create job descriptor from example data
    // 1. fill in hostnames
    int num_hosts1 = 3;
    const char** hosts1 = NORNS_PLIST_ALLOC(const char*, num_hosts1);

    for(int i=0; i<num_hosts1; ++i) {
        hosts1[i] = ex_hosts[i];
    }

    int num_hosts2 = 2;
    const char** hosts2 = NORNS_PLIST_ALLOC(const char*, num_hosts2);

    for(int i=0; i<num_hosts2; ++i) {
        hosts2[i] = ex_hosts[num_hosts1+i];
    }

    // 2. declare which backends the job is authorized to use
    // and provide info on them
    int num_backends = 3;
    struct norns_backend** backends = NORNS_PLIST_ALLOC(struct norns_backend*, num_backends); 

    for(int i=0; i<num_backends; ++i) {

        backends[i] = NORNS_ALLOC(sizeof(struct norns_backend));

        char str_mount[50];

        snprintf(str_mount, sizeof(str_mount), "/mnt/a-%d", i);
        size_t n = strlen(str_mount);

        backends[i]->b_mount = strndup(str_mount, n);
        backends[i]->b_type = NORNS_BACKEND_LOCAL_NVML;
        backends[i]->b_quota = 1024;
    }

    struct norns_job job = {
        .jb_hosts = hosts1,
        .jb_nhosts = num_hosts1,
        .jb_backends = backends,
        .jb_nbackends = num_backends
    };

    int rv;

    // register a job with ID 42
    if((rv = norns_register_job(&cred, 42, &job)) != NORNS_SUCCESS) {
            fprintf(stderr, "ERROR: norns_register_job failed: %s\n", norns_strerror(rv));
    }

    // register processes with access to this job
    for(int i=0; i<5; ++i) {
        if((rv = norns_add_process(&cred, 42, (uid_t) i, (gid_t) i, (pid_t) i)) != NORNS_SUCCESS) {
            fprintf(stderr, "norns_add_process failed: %s\n", norns_strerror(rv));
        }
    }


    // submit a task
    struct norns_iotd task = {
        .io_taskid = 0,
        .io_optype = 28,
        .io_src = {
            .in_type = NORNS_BACKEND_LOCAL_NVML,
            .in_path = {
//                .p_hostname = "node42",
                .p_hostname = NULL,
                .p_datapath = "nvm://foobar.bin",
            },
        },
        .io_dst = {
            .out_type = NORNS_BACKEND_LUSTRE,
            .out_path = {
                .p_hostname = "node42",
                .p_datapath = "nvm://baz.bin",
            }
        }
    };

    if((rv = norns_transfer(&task)) != NORNS_SUCCESS) {
        fprintf(stderr, "ERROR: norns_transfer failed: %s\n", norns_strerror(rv));
    }
    else {
        fprintf(stdout, "Task submitted (ID: %d)\n", task.io_taskid);
    }


    // unregister the job
    if((rv = norns_unregister_job(&cred, 42)) != NORNS_SUCCESS) {
        fprintf(stderr, "norns_unregister_job failed: %s\n", norns_strerror(rv));
    }

    NORNS_PLIST_FREE(hosts1);
    NORNS_PLIST_FREE(hosts2);
    NORNS_PLIST_FREE(backends);

}
