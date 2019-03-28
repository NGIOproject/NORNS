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

#define CATCH_CONFIG_RUNNER
#include "mpi-helpers.hpp"
#include "commands.hpp"
#include "catch.hpp"

bool
is_top_level_section(const Catch::SectionInfo& info) {
    const auto top_level_name("Given: ");
    return info.name.rfind(top_level_name, info.name.length()) !=
           std::string::npos;
}

struct TestListener : Catch::TestEventListenerBase {

    using TestEventListenerBase::TestEventListenerBase; // inherit constructor

    virtual void 
    sectionStarting(const Catch::SectionInfo& sectionInfo) override {

        (void) sectionInfo;

        MPI_TEST_RUN_IF(MPI_RANK_EQ(0)) {
            mpi::broadcast_command(server_command::restart);
            mpi::barrier(); // wait until servers finish starting up:w
        }
    }

    virtual void 
    sectionEnded(const Catch::SectionStats& sectionStats) override {
        (void) sectionStats;
    }

};

CATCH_REGISTER_LISTENER(TestListener)

int
main(int argc, char* argv[]) {

    mpi::initialize(&argc, &argv);

    int result = Catch::Session().run(argc, argv);

    MPI_TEST_RUN_IF(MPI_RANK_EQ(0)) {
        std::cerr << "at main()\n";
        mpi::barrier();
        mpi::broadcast_command(server_command::shutdown);
    }

    mpi::finalize();

    return result;
}
