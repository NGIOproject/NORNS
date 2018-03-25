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

#ifndef __NORNS_BACKENDS_H__
#define __NORNS_BACKENDS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Storage backend types */
//#define NORNS_BACKEND_LOCAL_NVML        0x10000000
//#define NORNS_BACKEND_REMOTE_NVML       0x10000001
//#define NORNS_BACKEND_PROCESS_MEMORY    0x10000005 // deprecated
#define NORNS_BACKEND_NVML              0x10000001
#define NORNS_BACKEND_LUSTRE            0x10000002
#define NORNS_BACKEND_ECHOFS            0x10000003
#define NORNS_BACKEND_POSIX_FILESYSTEM  0x10000004

#ifdef __cplusplus
}
#endif

#endif /* __NORNS_BACKENDS_H__ */
