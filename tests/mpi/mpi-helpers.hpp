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

#ifndef MPI_HELPERS_HPP
#define MPI_HELPERS_HPP

#include <string>
#include <vector>
#include <mpi.h>
#include "commands.hpp"

#define MPI_TEST_DEBUG

namespace mpi {

extern std::vector< std::pair<std::string, int> > test_hosts;

void
initialize(int* argc, char** argv[]);

void
finalize();

int
get_size();

int
get_rank();


server_command
broadcast_command(server_command cmd = server_command::accepting_commands);

void
barrier();

void
read_hosts(const std::string& filename);

} // namespace mpi

#define MPI_RANK_EQ(r) (mpi::get_rank() == r)
#define MPI_RANK_NEQ(r) (mpi::get_rank() != r)
#define MPI_TEST_RUN_IF(expr) if(expr)

#endif // MPI_HELPERS_HPP
