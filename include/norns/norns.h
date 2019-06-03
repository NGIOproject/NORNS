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
