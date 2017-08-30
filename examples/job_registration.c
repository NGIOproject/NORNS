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

    // create job descriptor from example data
    // 1. fill in hostnames
    int num_hosts = 3;
    const char** hosts = NORNS_PLIST_ALLOC(const char*, num_hosts);

    for(int i=0; i<num_hosts; ++i) {
        hosts[i] = ex_hosts[i];
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
        backends[i]->b_type = NORNS_LOCAL_NVML;
        backends[i]->b_quota = 1024;
    }

    struct norns_job job = {
        .jb_hosts = hosts,
        .jb_nhosts = num_hosts,
        .jb_backends = backends,
        .jb_nbackends = num_backends
    };


    struct norns_cred cred;

    int rv;

    // try to register a duplicate jobid
    for(int i=0; i<2; ++i) {
        if((rv = norns_register_job(&cred, 42, &job)) != NORNS_SUCCESS) {
            fprintf(stderr, "norns_register_job failed!\n");
        }
        else {
            fprintf(stdout, "norns_register_job succeded!\n");
        }
    }

    NORNS_PLIST_FREE(hosts);
    NORNS_PLIST_FREE(backends);

}
