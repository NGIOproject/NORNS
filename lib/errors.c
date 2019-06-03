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
