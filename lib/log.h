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

#ifndef __NORNS_LOG_H__
#define __NORNS_LOG_H__ 1

#include <stdio.h>

#ifdef __NORNS_DEBUG__
#define ERR(...)\
    log_error(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define DBG(...)\
    log_debug(__FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define ERR(...)
#define DBG(...)
#endif /* __NORNS_DEBUG__ */

#define FATAL(...)\
    log_fatal(__FILE__, __LINE__, __func__, __VA_ARGS__)

void log_init(const char* prefix, FILE* outfp);
void log_error(const char* file, int line, const char* func, 
               const char* fmt, ...);
void log_debug(const char* file, int line, const char* func, 
               const char* fmt, ...);
void log_fatal(const char* file, int line, const char* func, 
               const char* fmt, ...);

#endif /* __NORNS_LOG_H__ */
