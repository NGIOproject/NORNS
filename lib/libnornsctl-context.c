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
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "libnornsctl-context.h"

static pthread_key_t libnornsctl_context_key;

static int
libnornsctl_create_context_key(void) {

    int err = 0;

    if((err = pthread_key_create(&libnornsctl_context_key, NULL)) != 0) {
        errno = err;
        return -1;
    }

    return 0;
}

static void
libnornsctl_delete_context_key(void) {
    pthread_key_delete(libnornsctl_context_key);
}

void
libnornsctl_create_context(void) {

    struct libnornsctl_context* ctx;

    if(libnornsctl_create_context_key() != 0) {
        fprintf(stderr, "libnorns: Failed to create thread specific key: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    ctx = (struct libnornsctl_context*) pthread_getspecific(libnornsctl_context_key);

    if(ctx != NULL) {
        return;
    }

    ctx = (struct libnornsctl_context* ) malloc(sizeof(struct libnornsctl_context));

    if(ctx == NULL) {
        fprintf(stderr, "libnorns: Failed to allocate thread specific data\n");
        exit(EXIT_FAILURE);
    }

    memset(ctx, 0, sizeof(struct libnornsctl_context));

    int err = pthread_setspecific(libnornsctl_context_key, ctx);

    if(err != 0) {
        fprintf(stderr, "libnorns: Failed to set thread specific data\n");
        exit(EXIT_FAILURE);
    }
}

struct libnornsctl_context*
libnornsctl_get_context(void) {

    struct libnornsctl_context* ctx;

    ctx = (struct libnornsctl_context*) pthread_getspecific(libnornsctl_context_key);

    // this can only happen if we did not call libnornsctl_create_context()
    // before trying to access the context
    if(ctx == NULL) {
        fprintf(stderr, "libnorns: Failed to retrieve library context\n");
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void
libnornsctl_destroy_context(void) {
    libnornsctl_delete_context_key();
}
