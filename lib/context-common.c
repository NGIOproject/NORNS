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

    memset(ctx, 0, sizeof(libcontext_t));

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
