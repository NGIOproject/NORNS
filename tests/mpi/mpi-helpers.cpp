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

#include <stdexcept>
#include <stdlib.h>
#include "mpi-helpers.hpp"

#ifdef MPI_TEST_DEBUG
#include <iostream>
#include <sstream>
#endif // MPI_TEST_DEBUG


namespace mpi {

void
initialize(int* argc, char** argv[]) {
    if(::MPI_Init(argc, argv) != MPI_SUCCESS) {
        throw std::runtime_error("Failed to initialize MPI");
    }
}

void
finalize() {
    if(::MPI_Finalize() != MPI_SUCCESS) {
        throw std::runtime_error("Failed to finalize MPI");
    }
}


int
get_rank() {
    int world_rank;
    if(::MPI_Comm_rank(MPI_COMM_WORLD, &world_rank) != MPI_SUCCESS) {
        throw std::runtime_error("Failed to determine own rank");
    }

    return world_rank;
}

void
barrier() {

#ifdef MPI_TEST_DEBUG
    std::cerr << __PRETTY_FUNCTION__ << "\n";
    std::cerr << "Entering MPI_Barrier()\n";
#endif // MPI_TEST_DEBUG

    ::MPI_Barrier(MPI_COMM_WORLD);

#ifdef MPI_TEST_DEBUG
    std::cerr << "Exiting MPI_Barrier()\n";
#endif // MPI_TEST_DEBUG

}

server_command
broadcast_command(server_command cmd) {

    int c = static_cast<int>(cmd);

#ifdef MPI_TEST_DEBUG
    std::stringstream ss;
    ss << __PRETTY_FUNCTION__ << "(" << c << ")" << "\n";
    std::cerr << ss.str();
    std::cerr << "Entering MPI_Bcast()\n";
#endif // MPI_TEST_DEBUG

    ::MPI_Bcast(&c, 1, MPI_INT, 0, MPI_COMM_WORLD);

#ifdef MPI_TEST_DEBUG

    MPI_TEST_RUN_IF(MPI_RANK_NEQ(0)) {
        std::stringstream ss;
        ss << "command was " << c << "\n";
        std::cerr << ss.str();
    }
#endif

#ifdef MPI_TEST_DEBUG
    std::cerr << "Exiting MPI_Bcast()\n";
#endif // MPI_TEST_DEBUG

    return static_cast<server_command>(c);
}

} // namespace mpi
