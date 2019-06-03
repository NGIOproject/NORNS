/************************************************************************* 
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of NORNS, a service that allows other programs to   *
 * start, track and manage asynchronous transfers of data resources      *
 * between different storage backends.                                   *
 *                                                                       *
 * See AUTHORS file in the top level directory for information regarding *
 * developers and contributors.                                          *
 *                                                                       *
 * This software was developed as part of the EC H2020 funded project    *
 * NEXTGenIO (Project ID: 671951).                                       *
 *     www.nextgenio.eu                                                  *
 *                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining *
 * a copy of this software and associated documentation files (the       *
 * "Software"), to deal in the Software without restriction, including   *
 * without limitation the rights to use, copy, modify, merge, publish,   *
 * distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to *
 * the following conditions:                                             *
 *                                                                       *
 * The above copyright notice and this permission notice shall be        *
 * included in all copies or substantial portions of the Software.       *
 *                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                 *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS   *
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN    *
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN     *
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE      *
 * SOFTWARE.                                                             *
 *************************************************************************/

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

#if 0
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
            exit(EXIT_FAILURE);
        }
    }


    // update the job description
    job.jb_hosts = hosts2;
    job.jb_nhosts = num_hosts2;

    if((rv = norns_update_job(&cred, 42, &job)) != NORNS_SUCCESS) {
        fprintf(stderr, "norns_update_job failed: %s\n", norns_strerror(rv));
        exit(EXIT_FAILURE);
    }

    // unregister the job
    if((rv = norns_unregister_job(&cred, 42)) != NORNS_SUCCESS) {
        fprintf(stderr, "norns_unregister_job failed: %s\n", norns_strerror(rv));
        exit(EXIT_FAILURE);
    }

    NORNS_PLIST_FREE(hosts1);
    NORNS_PLIST_FREE(hosts2);
    NORNS_PLIST_FREE(backends);
#endif

}
