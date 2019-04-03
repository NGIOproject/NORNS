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
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <sstream>
#include "mpi-helpers.hpp"
#include "commands.hpp"
#include "catch.hpp"

namespace bfs = boost::filesystem;

struct TestListener : Catch::TestEventListenerBase {

    using TestEventListenerBase::TestEventListenerBase; // inherit constructor

    virtual void 
    sectionStarting(const Catch::SectionInfo& sectionInfo) override {
        (void) sectionInfo;
    }

    virtual void 
    sectionEnded(const Catch::SectionStats& sectionStats) override {
        (void) sectionStats;
    }
};

CATCH_REGISTER_LISTENER(TestListener)

int
main(int argc, char* argv[]) {

    Catch::Session session;

    std::string hostfile;

    auto cli = session.cli()
                | Catch::clara::Opt(hostfile, "filename")
                ["-H"]
                ["--hosts-file"]
                ("file containing the hosts running the test (one per line)")
                .required();

    session.cli(cli);

    int rv;
    if((rv = session.applyCommandLine(argc, argv)) != 0) {
        return rv;
    }

    if(hostfile.empty()) {
        std::cerr << "Missing host file.\n";
        return 1;
    }

    mpi::read_hosts(hostfile);

    mpi::initialize(&argc, &argv);

    rv = session.run(argc, argv);

    mpi::finalize();

    return rv;
}
