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

#ifndef __NORNS_LIB_H__
#define __NORNS_LIB_H__ 1

#ifndef NORNS_API_VERSION
#define NORNS_API_VERSION 10
#endif

#include <stdint.h>     /* For uint32_t et al. */
#include <stdbool.h>    /* For bool */
#include <time.h>       /* For struct timespec */

#include "norns_types.h"
#include "norns_error.h"

#ifdef __NORNS_DEBUG__
#include "norns_debug.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************/
/* Client API                                                             */
/**************************************************************************/

/* Initialize an asynchronous I/O task */
void 
norns_iotask_init(norns_iotask_t* task, 
                  norns_op_t operation,
                  norns_resource_t* src, 
                  norns_resource_t* dst) __THROW;

norns_iotask_t 
NORNS_IOTASK(norns_op_t operation, 
             norns_resource_t src, ...) __THROW;

/* Submit an asynchronous I/O task */
norns_error_t 
norns_submit(norns_iotask_t* task) __THROW;

/* wait for the completion of the I/O task associated to 'task' */
norns_error_t 
norns_wait(norns_iotask_t* task,
           const struct timespec* timeout) __THROW;

/* Try to cancel an asynchronous I/O task associated with task */
norns_error_t 
norns_cancel(norns_iotask_t* task) __THROW;

/* Check the status of a submitted I/O task */
norns_error_t 
norns_error(norns_iotask_t* task, 
            norns_stat_t* stats) __THROW;

/* Return a string describing the error number */
char* norns_strerror(norns_error_t errnum) __THROW;

#ifdef __cplusplus
}
#endif

#endif /* __NORNS_LIB_H__ */
