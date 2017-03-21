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
#include <boost/program_options.hpp>

#include "settings.hpp"
#include "urd.hpp"

namespace bpo = boost::program_options;

int main(int argc, char* argv[]){
    
    bool run_in_foreground = true;

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

    config_settings settings;

    settings.load("norns.json");

//    if (vm.count("f")) {
//        std::cout << "Compression level was set to " << vm["foreground"].as<bool>() << ".\n";
        std::cout << "Compression level was set to " << run_in_foreground << ".\n";
//    } else {
//        std::cout << "Compression level was not set.\n";
//    }

    exit(0);



    ::unlink(urd::SOCKET_FILE);

    urd().run();
}
