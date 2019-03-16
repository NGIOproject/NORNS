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

#include <norns.h>

#define ERR_REMAP(n) ((n) < 0 ? -(n) : (n))

const char* const norns_errlist[NORNS_ERRMAX + 1] = {
    [ERR_REMAP(NORNS_SUCCESS)] = "Success",
    [ERR_REMAP(NORNS_ESNAFU)] = "Internal error",
    [ERR_REMAP(NORNS_EBADREQUEST)] = "Bad request",
    [ERR_REMAP(NORNS_EBADARGS)] = "Bad arguments",
    [ERR_REMAP(NORNS_ENOMEM)] = "Cannot allocate memory",
    [ERR_REMAP(NORNS_ETIMEOUT)] = "Timeout exceeded",

    /* communication errors */
    [ERR_REMAP(NORNS_ECONNFAILED)] = "Cannot connect to daemon",
    [ERR_REMAP(NORNS_ERPCSENDFAILED)] = "Cannot send requests to daemon",
    [ERR_REMAP(NORNS_ERPCRECVFAILED)] = "Cannot receive responses from daemon",
    [ERR_REMAP(NORNS_EACCEPTPAUSED)] = "Daemon does not accept new tasks",

    /* job errors */
    [ERR_REMAP(NORNS_EJOBEXISTS)] = "Job already exists",
    [ERR_REMAP(NORNS_ENOSUCHJOB)] = "Job does not exist",

    /* process errors */
    [ERR_REMAP(NORNS_EPROCESSEXISTS)] = "Process already exists",
    [ERR_REMAP(NORNS_ENOSUCHPROCESS)] = "Process does not exist",
    
    /* backend errors */
    [ERR_REMAP(NORNS_ENAMESPACEEXISTS)] = "Namespace already exists",
    [ERR_REMAP(NORNS_ENOSUCHNAMESPACE)] = "Namespace does not exist",
    [ERR_REMAP(NORNS_ENAMESPACENOTEMPTY)] = "Namespace is not empty",
    
    /* task errors */
    [ERR_REMAP(NORNS_ETASKEXISTS)] = "Task already exists",
    [ERR_REMAP(NORNS_ENOSUCHTASK)] = "Task does not exist",
    [ERR_REMAP(NORNS_ETOOMANYTASKS)] = "Too many pending tasks",
    [ERR_REMAP(NORNS_ETASKSPENDING)] = "There are still pending tasks",

    /* resource errors */
    [ERR_REMAP(NORNS_ERESOURCEEXISTS)] = "Resource already exists",
    [ERR_REMAP(NORNS_ENOSUCHRESOURCE)] =  "Resource does not exist",

    /* misc errors */
    [ERR_REMAP(NORNS_ENOTSUPPORTED)] = "Not supported",
    [ERR_REMAP(NORNS_ESYSTEMERROR)] = "Operating system error",

    /* fallback */
    [ERR_REMAP(NORNS_ERRMAX)] = "Unknown error",

};

char*
__norns_strerror(int errnum) {

    if(errnum > NORNS_ERRMAX) {
        errnum = NORNS_ERRMAX;
    }

    return (char*) norns_errlist[ERR_REMAP(errnum)];
}

# define weak_alias(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((weak, alias (#name)));

weak_alias(__norns_strerror, norns_strerror);
weak_alias(__norns_strerror, nornsctl_strerror);
