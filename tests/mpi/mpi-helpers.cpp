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
#include <boost/regex.hpp>
#include <boost/filesystem/fstream.hpp>
#include <stdlib.h>
#include "mpi-helpers.hpp"

#ifdef MPI_TEST_DEBUG
#include <iostream>
#include <sstream>
#endif // MPI_TEST_DEBUG

namespace bfs = boost::filesystem;

namespace mpi {

std::vector< std::pair<std::string, int> > test_hosts;

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
get_size() {
    int world_size;

    if(::MPI_Comm_size(MPI_COMM_WORLD, &world_size) != MPI_SUCCESS) {
        throw std::runtime_error("Failed to determine MPI_COMM_WORLD size");
    }

    return world_size;
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

    if(::MPI_Barrier(MPI_COMM_WORLD) != MPI_SUCCESS) {
        throw std::runtime_error("MPI_Barrier()");
    }

#ifdef MPI_TEST_DEBUG
    std::cerr << "Exiting MPI_Barrier()\n";
#endif // MPI_TEST_DEBUG

}

server_command
broadcast_command(server_command cmd) {

    int c = static_cast<int>(cmd);

#ifdef MPI_TEST_DEBUG
    std::stringstream ss;
    ss << __PRETTY_FUNCTION__ << "(" << server_command_names[c] << ")" << "\n";
    std::cerr << ss.str();
    std::cerr << "Entering MPI_Bcast()\n";
#endif // MPI_TEST_DEBUG

    if(::MPI_Bcast(&c, 1, MPI_INT, 0, MPI_COMM_WORLD) != MPI_SUCCESS) {
        throw std::runtime_error("MPI_Bcast()");
    }

#ifdef MPI_TEST_DEBUG

    MPI_TEST_RUN_IF(MPI_RANK_NEQ(0)) {
        std::stringstream ss;
        ss << "command was " << server_command_names[c] << "\n";
        std::cerr << ss.str();
    }
#endif

#ifdef MPI_TEST_DEBUG
    std::cerr << "Exiting MPI_Bcast()\n";
#endif // MPI_TEST_DEBUG

    return static_cast<server_command>(c);
}

void
read_hosts(const std::string& filename) {

    bfs::ifstream ifs(filename);

    std::stringstream ss;

    std::string line;
    while(std::getline(ifs, line)) {

        // ignore comments
        if(boost::regex_match(line, boost::regex("^\\s*?#\\s*?.*$"))) {
            continue;
        }

        boost::match_results<std::string::const_iterator> captures;

        // parse host
        if(boost::regex_match(
                    line, 
                    captures,
                    boost::regex("^\\s*?(\\S*?)\\s*?:\\s*?(\\d+)\\s*$"))) {
            test_hosts.emplace_back(
                    std::make_pair(captures[1], std::stoi(captures[2])));
        }
    }
}

} // namespace mpi
