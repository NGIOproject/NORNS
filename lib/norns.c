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

#include "norns.h"
#include "nornsctl.h"

#include "log.h"
#include "xmalloc.h"
#include "communication.h"

#define LIBNORNS_LOG_PREFIX "libnorns"
#define MIN_WAIT_TIME ((useconds_t) 250*1e3)

__attribute__((constructor))
static void
libnornsctl_init(void) {
    log_init(LIBNORNS_LOG_PREFIX);
}

/* Public API */

norns_iotask_t
NORNS_IOTASK(norns_op_t optype, norns_resource_t src, norns_resource_t dst) {
    norns_iotask_t task;

    norns_iotask_init(&task, optype, &src, &dst);

    return task;
}

void norns_iotask_init(norns_iotask_t* task, norns_op_t optype,
                     norns_resource_t* src, norns_resource_t* dst) {

    if(task == NULL) {
        return;
    }

    if(src == NULL || dst == NULL) {
        memset(task, 0, sizeof(*task));
        return;
    }

    task->t_id = 0;
    task->t_op = optype;
    task->t_src = *src;
    task->t_dst = *dst;
}

norns_error_t
norns_submit(norns_iotask_t* task) {

    if(task == NULL) {
        return NORNS_EBADARGS;
    }

    return send_submit_request(task);
}

norns_error_t
norns_status(norns_iotask_t* task, norns_stat_t* stats) {

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

        if(stats.st_status == NORNS_EFINISHED) {
            return NORNS_SUCCESS;
        }

        // wait for 250 milliseconds
        // before retrying
        usleep(MIN_WAIT_TIME);
    } while(true);

    return NORNS_SUCCESS;
}
