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
