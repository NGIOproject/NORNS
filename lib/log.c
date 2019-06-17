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

#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "log.h"

static const char* log_prefix;
static FILE* log_outfp;

#define MAX_LOG_MSG 8192
#define MAX_ERROR_MSG 128

void
log_init(const char* prefix, FILE* outfp) {

    static bool once = false;

    if(once) {
        return;
    }

    log_prefix = prefix;
    log_outfp = outfp;

    once = true;
}

static void
log_helper(const char* file, int line, const char* func, const char* prefix,
           const char* suffix, const char* fmt, va_list ap) {

    int saved_errno = errno;
    char buffer[MAX_LOG_MSG];
    char errstr[MAX_ERROR_MSG] = "";
    unsigned cc = 0;
    const char* sep = "";
    int ret;

    if(log_outfp == NULL) {
        return;
    }

    if(file != NULL) {

        char* f;

        while((f = strrchr(file, '/')) != NULL) {
            file = f+1;
        }

        ret = snprintf(&buffer[cc], MAX_LOG_MSG,
                "<%s>: [%s:%d %s()] %s: ", log_prefix, file, line, func, prefix);

        if(ret < 0) {
            fprintf(log_outfp, "vsnprintf failed");
            goto out;
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
            fprintf(log_outfp, "vsnprintf failed");
            goto out;
        }

        cc += (unsigned) ret;
    }

    snprintf(&buffer[cc], MAX_LOG_MSG - cc, "%s%s%s", sep, errstr, suffix);
    fprintf(log_outfp, "%s", buffer);

out:
	errno = saved_errno;
}

void
log_error(const char* file, int line, const char* func, 
          const char* fmt, ...) {

    va_list ap;
    va_start(ap, fmt);
    log_helper(file, line, func, "ERROR", "\n", fmt, ap);
    va_end(ap);
}

void
log_debug(const char* file, int line, const char* func, 
          const char* fmt, ...) {

    va_list ap;
    va_start(ap, fmt);
    log_helper(file, line, func, "DEBUG", "\n", fmt, ap);
    va_end(ap);
}

void
log_fatal(const char* file, int line, const char* func, 
          const char* fmt, ...) {

    va_list ap;
    va_start(ap, fmt);
    log_helper(file, line, func, "FATAL", "\n", fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}
