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

#ifndef __NORNS_LIB_H__
#define __NORNS_LIB_H__ 1

#include <features.h>
#include <sys/types.h>

__BEGIN_DECLS

/* I/O task descriptor */
struct norns_iotd {
    int ni_tid;           /* task identifier */
    int ni_ibid;          /* source backend identifier */
    const char* ni_ipath; /* path to data source */
    int ni_obid;          /* destination backend identifier */
    const char* ni_opath; /* path to data destination */
    int ni_type;          /* operation to be performed */
};

/* Task types */
enum {
    NORNS_COPY,
    NORNS_MOVE
};


/* I/O task status descriptor */
struct norns_iotst {
    int ni_status;  /* current status */
};

/* Status codes */
enum {
    NORNS_WAITING,
    NORNS_INPROGRESS,
    NORNS_COMPLETE
};


void norns_init() __THROW;

int norns_getconf() __THROW;

/* Enqueue an asynchronous I/O task */
int norns_transfer(struct norns_iotd* iotdp) __THROW;

/* Try to cancel an asynchronous I/O task associated with iotdp */
ssize_t norns_cancel(struct norns_iotd* iotdp) __THROW;

/* Retrieve return status associated with iotdp */
ssize_t norns_return(struct norns_iotd* iotdp) __THROW;

/* Retrieve current status associated with iotdp */
ssize_t norns_progress(struct norns_iotd* iotdp, struct norns_iotst* statp) __THROW;

/* Retrieve status associated with all current tasks */
ssize_t norns_progress_all(struct norns_iotst** statp) __THROW;

/* Retrieve error status associated with iotdp */
int norns_error(struct norns_iotd* iotdp) __THROW;

__END_DECLS

#endif /* __NORNS_LIB_H__ */
