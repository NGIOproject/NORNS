//
// Copyright (C) 2017 Barcelona Supercomputing Center
//                    Centro Nacional de Supercomputacion
//
// This file is part of the Data Scheduler, a daemon for tracking and managing
// requests for asynchronous data transfer in a hierarchical storage environment.
//
// See AUTHORS file in the top level directory for information
// regarding developers and contributors.
//
// The Data Scheduler is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Data Scheduler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Data Scheduler.  If not, see <http://www.gnu.org/licenses/>.
//

#include <unistd.h>
#include <sys/types.h>
#include <boost/program_options.hpp>

#include "settings.hpp"
#include "urd.hpp"

namespace bpo = boost::program_options;

int main(int argc, char* argv[]){
    
    bool run_in_foreground = !defaults::daemonize;

    // declare a group of options that will be allowed only on the command line
    bpo::options_description generic("Allowed options");
    generic.add_options()
        (",f", bpo::bool_switch(&run_in_foreground), "foreground operation") // check how to do flags
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
    if(stat(defaults::config_file, &stbuf) != 0) {
        std::cerr << "Missing configuration file '" << defaults::config_file << "'\n";
        exit(EXIT_FAILURE);
    }

    config_settings settings;

    settings.load(defaults::config_file);

    settings.m_daemonize = !run_in_foreground;

    urd daemon;
    daemon.set_configuration(settings);
    daemon.run();

    exit(EXIT_SUCCESS);
}
