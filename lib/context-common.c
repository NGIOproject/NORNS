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

#include "context-common.h"

static pthread_key_t LIBSYMBOL(context_key);

static int
LIBSYMBOL(create_context_key)(void) {

    int err = 0;

    if((err = pthread_key_create(&LIBSYMBOL(context_key), NULL)) != 0) {
        errno = err;
        return -1;
    }

    return 0;
}

static void
LIBSYMBOL(delete_context_key)(void) {
    pthread_key_delete(LIBSYMBOL(context_key));
}

void
LIBSYMBOL(create_context)(void) {


    if(LIBSYMBOL(create_context_key)() != 0) {
        fprintf(stderr, "libnorns: Failed to create thread specific key: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    libcontext_t* ctx = (libcontext_t*) 
        pthread_getspecific(LIBSYMBOL(context_key));

    if(ctx != NULL) {
        return;
    }

    ctx = malloc(sizeof(libcontext_t));

    if(ctx == NULL) {
        fprintf(stderr, "libnorns: Failed to allocate thread specific data\n");
        exit(EXIT_FAILURE);
    }

    int err = pthread_setspecific(LIBSYMBOL(context_key), ctx);

    if(err != 0) {
        fprintf(stderr, "libnorns: Failed to set thread specific data\n");
        exit(EXIT_FAILURE);
    }
}

libcontext_t*
LIBSYMBOL(get_context)(void) {

    libcontext_t* ctx;

    ctx = (libcontext_t*) 
        pthread_getspecific(LIBSYMBOL(context_key));

    // this can only happen if we did not call LIBSYMBOL(create_context)()
    // before trying to access the context
    if(ctx == NULL) {
        fprintf(stderr, "libnorns: Failed to retrieve library context\n");
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void
LIBSYMBOL(destroy_context)(void) {

    libcontext_t* ctx = (libcontext_t*) 
        pthread_getspecific(LIBSYMBOL(context_key));

    if(ctx != NULL) {
        free(ctx);
    }

    LIBSYMBOL(delete_context_key)();
}
