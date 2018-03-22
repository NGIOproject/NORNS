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
#include <sys/types.h>
#include <boost/program_options.hpp>

#include "settings.hpp"
#include "urd.hpp"

namespace bpo = boost::program_options;

int main(int argc, char* argv[]){
    
    bool run_in_foreground = !norns::defaults::daemonize;
    bool dry_run = norns::defaults::dry_run;

    // declare a group of options that will be allowed only on the command line
    bpo::options_description generic("Allowed options");
    generic.add_options()
        (",f", bpo::bool_switch(&run_in_foreground), "foreground operation") // check how to do flags
        ("dry-run,d", bpo::bool_switch(&dry_run),    "don't actually execute tasks") // check how to do flags
        ("version,v",                                "print version string")
        ("help,h",                                   "produce help message")
    ;

    // declare a group of options that will be allowed in a config file
    bpo::variables_map vm;
    bpo::store(bpo::parse_command_line(argc, argv, generic), vm);
    bpo::notify(vm);    

    if (vm.count("help")) {
        std::cout << generic << "\n";
        exit(EXIT_SUCCESS);
    }

    struct stat stbuf;
    if(stat(norns::defaults::config_file, &stbuf) != 0) {
        std::cerr << "Missing configuration file '" << norns::defaults::config_file << "'\n";
        exit(EXIT_FAILURE);
    }

    norns::config_settings settings;

    settings.load(norns::defaults::config_file);

    settings.m_daemonize = !run_in_foreground;
    settings.m_dry_run = dry_run;

    norns::urd daemon;
    daemon.configure(settings);

    int status = daemon.run();

    return status;
}
