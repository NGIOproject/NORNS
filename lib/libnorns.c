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
#define MIN_WAIT_TIME ((useconds_t) 250*1e3)

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

    return send_status_request(task, stats);
}


/* wait for the completion of the I/O task associated to 'task' */
norns_error_t
norns_wait(norns_iotask_t* task) {

    norns_error_t rv;
    norns_stat_t stats;

    if(task == NULL) {
        ERR("invalid arguments");
        return NORNS_EBADARGS;
    }

    do {
        rv = send_status_request(task, &stats);

        if(rv != NORNS_SUCCESS) {
            ERR("error waiting for request: %s", norns_strerror(rv));
            return rv;
        }

        if(stats.st_status == NORNS_EFINISHED || 
           stats.st_status == NORNS_EFINISHEDWERROR) {
            return NORNS_SUCCESS;
        }

        // wait for 250 milliseconds
        // before retrying
        usleep(MIN_WAIT_TIME);
    } while(true);

    return NORNS_SUCCESS;
}
