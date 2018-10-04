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
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "config.hpp"
#include "urd.hpp"

namespace bfs = boost::filesystem;
namespace bpo = boost::program_options;

namespace {

void 
option_dependency(const boost::program_options::variables_map& vm,
                  const std::string& for_what, 
                  const std::string& required_option) {

    if(vm.count(for_what) && !vm[for_what].defaulted()) {
        if(vm.count(required_option) == 0 || vm[required_option].defaulted()) {
            throw std::logic_error(std::string("Option '") + for_what +
                  "' requires option '" + required_option + "'.");
        }
    }
}

}

int main(int argc, char* argv[]){

    norns::config::settings cfg;
    cfg.load_defaults();
    
    bool run_in_foreground = !cfg.daemonize();
    bool dry_run = cfg.dry_run();
    uint32_t dry_run_duration = cfg.dry_run_duration();

    // declare a group of options that will be allowed only on the command line
    bpo::options_description generic("Allowed options");
    generic.add_options()
        (",f", 
         bpo::bool_switch(&run_in_foreground),
         "foreground operation")
        ("dry-run,d", 
         bpo::value<uint32_t>()->value_name("N")->implicit_value(100),
         "don't actually execute tasks, but wait N microseconds per task if an argument is provided")
        ("version,v",
         "print version string")
        ("help,h",
         "produce help message")
    ;

    // declare a group of options that will be allowed in a config file
    bpo::variables_map vm;
    bpo::store(bpo::parse_command_line(argc, argv, generic), vm);
    bpo::notify(vm);

    if (vm.count("help")) {
        std::cout << generic << "\n";
        exit(EXIT_SUCCESS);
    }

    if(!bfs::exists(cfg.config_file())) {
        std::cerr << "Failed to access service configuration file " << cfg.config_file() << "\n";
        exit(EXIT_FAILURE);
    }

    try {
        cfg.load_from_file(cfg.config_file());
    }
    catch(const std::exception& ex) {
        std::cerr << "Error reading config file:\n    " << ex.what() << "\n";
        exit(EXIT_FAILURE);
    }

    if(vm.count("dry-run")) {
        dry_run = true;
        dry_run_duration = vm["dry-run"].as<uint32_t>();
    }

    // override settings from file with command-line arguments
    cfg.daemonize() = !run_in_foreground;
    cfg.dry_run() = dry_run;
    cfg.dry_run_duration() = dry_run_duration;

    norns::urd daemon;
    daemon.configure(cfg);

    return daemon.run();
}
