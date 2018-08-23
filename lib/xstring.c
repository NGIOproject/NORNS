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

#include <unistd.h>
#include <string.h>
#include "xmalloc.h"
#include "xstring.h"


char*
xstrdup(const char* str) {

    size_t sz;
    char* res;

    if(str == NULL) {
        return NULL;
    }

    sz = XMIN(strlen(str) + 1, XMAX_STRING_LENGTH);
    res = (char*) xmalloc(sz);
    strncpy(res, str, sz);
    res[sz-1] = '\0';

    return res;
}

int
xstrcnmp(const char* s1, const char* s2) {

    if(s1 == NULL && s2 == NULL) {
        return 0;
    }
    else if(s1 == NULL) {
        return -1;
    }
    else if(s2 == NULL) {
        return 1;
    }
    else {
        size_t sz;
        sz = XMIN(strlen(s1), strlen(s2));
        sz = XMIN(sz, XMAX_STRING_LENGTH);

        return strncmp(s1, s2, sz);
    }
    
}

