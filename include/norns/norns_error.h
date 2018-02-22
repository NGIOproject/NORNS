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


#if !defined(__NORNS_LIB_H__)
#error "Never include <norns_error.h> directly; use <norns.h> instead."
#endif

#ifndef __NORNS_ERROR_H__
#define __NORNS_ERROR_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

typedef int norns_error_t;

#define NORNS_ERRMAX 512

/** Error codes */
#define NORNS_SUCCESS           0
#define NORNS_ESNAFU            -1
#define NORNS_EBADARGS          -2
#define NORNS_EBADREQUEST       -3
#define NORNS_ENOMEM            -4
#define NORNS_ECONNFAILED       -5
#define NORNS_ERPCSENDFAILED    -6
#define NORNS_ERPCRECVFAILED    -7
#define NORNS_EJOBEXISTS        -8
#define NORNS_ENOSUCHJOB        -9
#define NORNS_EPROCESSEXISTS    -10
#define NORNS_ENOSUCHPROCESS    -11
#define NORNS_EBACKENDEXISTS    -12
#define NORNS_ENOSUCHBACKEND    -13

#define NORNS_ENOTSUPPORTED     -64

#ifdef __cplusplus
}
#endif

#endif /* __NORNS_ERROR_H__ */
