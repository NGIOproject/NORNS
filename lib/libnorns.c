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
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

#include "norns.h"
#include "nornsctl.h"
#include "libnorns-context.h"

#include "config-parser.h"
#include "log.h"
#include "xmalloc.h"
#include "xstring.h"
#include "communication.h"
#include "defaults.h"

#define LIBNORNS_LOG_PREFIX "libnorns"

// default timeout is 250 microseconds
const struct timespec LIBNORNS_DEFAULT_TIMEOUT = {
    .tv_sec = 0,
    .tv_nsec = (250L*1e3L)
};

static void
load_config_file(void) {

    const char* config_file = NULL;

    if((config_file = getenv("NORNS_CONFIG_FILE")) == NULL) {
        config_file = norns_default_config_file;
    }

    DBG("Loading configuration file \"%s\"", config_file);

    if(access(config_file, R_OK) == -1) {
        FATAL("!Failed to access norns configuration file");
    }

    struct kvpair valid_opts[] = {
        { "global_socket",  NULL },
    };

    size_t num_valid_opts = sizeof(valid_opts) / sizeof(valid_opts[0]);

    if(parse_config_file(config_file, valid_opts, num_valid_opts) != 0) {
        FATAL("Failed to parse norns configuration file"); 
    }

    struct libnorns_context* ctx = libnorns_get_context();

    if(ctx == NULL) {
        FATAL("Failed to retrieve library context");
    }
    
    if(ctx->config_file != NULL) {
        xfree(ctx->config_file);
    }

    ctx->config_file = xstrdup(config_file);

    if(ctx->api_socket != NULL) {
        xfree(ctx->api_socket);
    }

    ctx->api_socket = xstrdup(valid_opts[0].val);

    DBG("Configuration loaded from file:");
    DBG("   user api socket: %s", ctx->api_socket);

    xfree(valid_opts[0].val);
}

__attribute__((constructor))
static void
libnorns_init(void) {

    FILE* logfp = stderr;

#ifdef __NORNS_DEBUG__
    bool override_config_file = false;

    // parse relevant environment variables
    if(getenv("NORNS_DEBUG_OUTPUT_TO_STDERR") == NULL) {
        logfp = NULL;
        DBG("NORNS_DEBUG_OUTPUT_TO_STDERR is set, all messages will be "
            "sent to stderr");
    }

    if(getenv("NORNS_DEBUG_CONFIG_FILE_OVERRIDE") != NULL) {
        override_config_file = true;
        DBG("NORNS_DEBUG_CONFIG_FILE_OVERRIDE is set, default configuration "
            "file will be ignored");
    }
#endif

    log_init(LIBNORNS_LOG_PREFIX, logfp);
    libnorns_create_context();

#ifdef __NORNS_DEBUG__
    if(override_config_file) {
        return;
    }
#endif

    load_config_file();
}

__attribute__((destructor))
static void
libnorns_fini(void) {
    libnorns_destroy_context();
}

#ifdef __NORNS_DEBUG__
void
libnorns_reload_config_file(void) {
    DBG("Reloading configuration file");
    load_config_file();
}
#endif

/* Public API */

norns_iotask_t
NORNS_IOTASK(norns_op_t optype, norns_resource_t src, ...) {
    norns_iotask_t task;

    if(optype == NORNS_IOTASK_REMOVE) {
        norns_iotask_init(&task, optype, &src, NULL);
        return task;
    }

    va_list ap;
    va_start(ap, src);
    norns_resource_t dst = va_arg(ap, norns_resource_t);
    norns_iotask_init(&task, optype, &src, &dst);
    va_end(ap);

    return task;
}

void 
norns_iotask_init(norns_iotask_t* task, norns_op_t optype,
                  norns_resource_t* src, norns_resource_t* dst) {

    if(task == NULL) {
        return;
    }

    memset(task, 0, sizeof(*task));

    if(src == NULL) {
        return;
    }

    task->t_id = 0;
    task->t_op = optype;
    task->t_src = *src;

    if(dst != NULL) {
        task->t_dst = *dst;
        return;
    }

    // dst is NULL, set r_flags so that we are aware of it later
    task->t_dst.r_flags = NORNS_NULL_RESOURCE;
}

norns_error_t
norns_submit(norns_iotask_t* task) {

    if(task == NULL) {
        return NORNS_EBADARGS;
    }

    return send_submit_request(task);
}

norns_error_t
norns_error(norns_iotask_t* task, norns_stat_t* stats) {

    if(task == NULL || stats == NULL) {
        return NORNS_EBADARGS;
    }

    // we might already have the task status cached in the task descriptor if
    // the user called norns_wait() and the task completed
    if(task->__t_status.st_status == NORNS_EFINISHED ||
       task->__t_status.st_status == NORNS_EFINISHEDWERROR) {
        *stats = task->__t_status;
        return NORNS_SUCCESS;
    }

    return send_status_request(task, stats);
}


/* wait for the completion of the I/O task associated to 'task' */
norns_error_t
norns_wait(norns_iotask_t* task,
           const struct timespec* timeout) {

    norns_error_t rv;
    norns_stat_t stats;

    if(task == NULL) {
        ERR("invalid arguments");
        return NORNS_EBADARGS;
    }

    // if user provided a timeout, suspend this thread for 
    // (at least) the amount of time requested
    struct timespec requested = 
        timeout == NULL ? 
            LIBNORNS_DEFAULT_TIMEOUT :
            *timeout;
    struct timespec remaining;

    do {
        // check if task has finished
        rv = send_status_request(task, &stats);

        if(rv != NORNS_SUCCESS) {
            ERR("error waiting for request: %s", norns_strerror(rv));
            return rv;
        }

        if(stats.st_status == NORNS_EFINISHED || 
           stats.st_status == NORNS_EFINISHEDWERROR) {
            // given that the task finished, we can save its completion status 
            // so that future calls to norns_error() can retrieve it without
            // having to contact the server
            task->__t_status = stats;
            return NORNS_SUCCESS;
        }

        // task not finished: sleep
        if(nanosleep(&requested, &remaining) != 0) {
            if(errno != EINTR) {
                ERR("nanosleep error: %s", norns_strerror(rv));
                if(errno == EINVAL) {
                    return NORNS_EBADARGS;
                }
                return NORNS_ESNAFU;
            }

            // EINTR: we've been interrupted, check the task status again just
            // in case it's finished and we can avoid sleeping again
            requested = 
                timeout == NULL ? 
                    LIBNORNS_DEFAULT_TIMEOUT :
                    remaining;
            // fall through
        }

        if(timeout == NULL) {
            continue;
        }

        return NORNS_ETIMEOUT;

    } while(true);

    return NORNS_SUCCESS;
}
