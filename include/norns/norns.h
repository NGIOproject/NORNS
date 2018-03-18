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

#include <sys/types.h>
#include <stdint.h>
#include <assert.h>

#include "norns_types.h"
#include "norns_error.h"
#include "norns_backends.h"
#include "norns_resources.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Descriptor for an I/O task */
typedef struct {
    norns_tid_t         t_id;   /* task identifier */
    norns_op_t          t_op;   /* operation to be performed */
    norns_resource_t    t_src;  /* source resource */
    norns_resource_t    t_dst;  /* destination resource */
} norns_iotask_t;

/* Task types */
#define NORNS_IOTASK_COPY 0x1
#define NORNS_IOTASK_MOVE 0x2

/* I/O task status descriptor */
typedef struct {
    norns_status_t st_status;    /* task current status */
    norns_error_t  st_error;     /* task return value */
    size_t         st_pending;   /* bytes pending in task */
    size_t         st_total;     /* total bytes in task */
} norns_stat_t;

/**************************************************************************/
/* Client API                                                             */
/**************************************************************************/

/* Initialize an asynchronous I/O task */
void norns_iotask_init(norns_iotask_t* task, norns_op_t operation,
                       norns_resource_t* src, norns_resource_t* dst) __THROW;

norns_iotask_t NORNS_IOTASK(norns_op_t operation, norns_resource_t src, 
                            norns_resource_t dst) __THROW;

/* Submit an asynchronous I/O task */
norns_error_t norns_submit(norns_iotask_t* task) __THROW;

/* wait for the completion of the I/O task associated to 'task' */
norns_error_t norns_wait(norns_iotask_t* task) __THROW;

/* Try to cancel an asynchronous I/O task associated with task */
norns_error_t norns_cancel(norns_iotask_t* task) __THROW;

/* Check the status of a submitted I/O task */
norns_error_t norns_status(norns_iotask_t* task, norns_stat_t* stats) __THROW;

/* Retrieve return status associated with task */
norns_error_t norns_return(norns_iotask_t* task, norns_stat_t* stats) __THROW;

/* Retrieve current status associated with task (if task is NULL, retrieve status for all running tasks) */
//ssize_t norns_progress(norns_iotask_t* task, struct norns_iotst* statp) __THROW;

/* Retrieve error status associated with task */
//int norns_error(norns_iotask_t* task) __THROW;

/* Return a string describing the error number */
char* norns_strerror(norns_error_t errnum) __THROW;

#ifdef __cplusplus
}
#endif

#endif /* __NORNS_LIB_H__ */
