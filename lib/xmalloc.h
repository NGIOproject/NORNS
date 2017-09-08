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

#ifndef __XMALLOC_H__
#define __XMALLOC_H__

#include <sys/types.h>
#include <stdbool.h>

#pragma GCC visibility push(hidden)

#define xmalloc(__sz) \
    norns_xmalloc(__sz, true)

#define xmalloc_nz(__sz) \
    norns_xmalloc(__sz, false)

#define xfree(__p) \
    norns_xfree((void**)&(__p))

void* norns_xmalloc(size_t, bool);
void norns_xfree(void**);

#pragma GCC visibility pop

#endif /* __XMALLOC_H__ */
