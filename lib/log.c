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

#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "log.h"

static const char* log_prefix;

#define MAX_LOG_MSG 8192
#define MAX_ERROR_MSG 128

void
log_init(const char* prefix) {

    static bool once = false;

    if(once) {
        return;
    }

    log_prefix = prefix;

    once = true;
}

static void
log_error_helper(const char* file, int line, const char* func, 
                 const char* suffix, const char* fmt, va_list ap) {

    int saved_errno = errno;
    char buffer[MAX_LOG_MSG];
    char errstr[MAX_ERROR_MSG] = "";
    unsigned cc = 0;
    const char* sep = "";
    int ret;

    if(file != NULL) {

        char* f;

        while((f = strchr(file, '/')) != NULL) {
            file = f+1;
        }

        ret = snprintf(&buffer[cc], MAX_LOG_MSG - cc,
                "<%s>: [%s:%d %s] ", log_prefix, file, line, func);

        if(ret < 0) {
            fprintf(stderr, "vsnprintf failed");
            goto end;
        }

        cc += (unsigned) ret;
    }

    if(fmt != NULL) {

        // if fmt begins with '!', append also the system's error message
        if(*fmt == '!') {
            ++fmt;
            sep = ": ";
            strerror_r(errno, errstr, MAX_ERROR_MSG);
        }

        ret = vsnprintf(&buffer[cc], MAX_LOG_MSG - cc, fmt, ap);

        if(ret < 0) {
            fprintf(stderr, "vsnprintf failed");
            goto end;
        }

        cc += (unsigned) ret;
    }

    snprintf(&buffer[cc], MAX_LOG_MSG - cc, "%s%s%s", sep, errstr, suffix);

    fprintf(stderr, "%s", buffer);

end:
	errno = saved_errno;
}

void
log_error(const char* file, int line, const char* func, 
          const char* fmt, ...) {

    va_list ap;
    va_start(ap, fmt);
    log_error_helper(file, line, func, "\n", fmt, ap);
    va_end(ap);
}
