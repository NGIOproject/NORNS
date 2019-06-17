/************************************************************************* 
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of NORNS, a service that allows other programs to   *
 * start, track and manage asynchronous transfers of data resources      *
 * between different storage backends.                                   *
 *                                                                       *
 * See AUTHORS file in the top level directory for information regarding *
 * developers and contributors.                                          *
 *                                                                       *
 * This software was developed as part of the EC H2020 funded project    *
 * NEXTGenIO (Project ID: 671951).                                       *
 *     www.nextgenio.eu                                                  *
 *                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining *
 * a copy of this software and associated documentation files (the       *
 * "Software"), to deal in the Software without restriction, including   *
 * without limitation the rights to use, copy, modify, merge, publish,   *
 * distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to *
 * the following conditions:                                             *
 *                                                                       *
 * The above copyright notice and this permission notice shall be        *
 * included in all copies or substantial portions of the Software.       *
 *                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                 *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS   *
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN    *
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN     *
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE      *
 * SOFTWARE.                                                             *
 *************************************************************************/

#include <unistd.h>
#include <sys/types.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include "config.hpp"
#include "urd.hpp"
#include "fmt.hpp"

namespace bfs = boost::filesystem;
namespace bpo = boost::program_options;

void
print_version(const std::string& progname) {
    fmt::print("{} {}\n", progname, NORNS_VERSION);
}

void 
print_help(const std::string& progname, 
           const bpo::options_description& opt_desc) {
    fmt::print("Usage: {} [options]\n\n", progname);
    fmt::print("{}", opt_desc);
}

int
main(int argc, char* argv[]) {

    norns::config::settings cfg;
    cfg.load_defaults();
    
    // define the command line options allowed
    bpo::options_description opt_desc("Options");
    opt_desc.add_options()
        // run in foreground
        (",f", 
         bpo::bool_switch()
            ->default_value(false)
            ->notifier(
                [&](const bool& flag_value) {
                    cfg.daemonize(!flag_value);
                }),
         "foreground operation")

        // noop tasks + duration
        ("dry-run,d", 
         bpo::value<uint32_t>()
            ->value_name("N")
            ->implicit_value(100)
            ->notifier(
                [&](const uint32_t& duration_value) {
                    cfg.dry_run(true);
                    cfg.dry_run_duration(duration_value);
                }),
         "don't actually execute tasks, but wait N microseconds per task if "
         "an argument is provided")

        // force logging messages to the console
        ("force-console,C",
         bpo::value<std::string>()
            ->implicit_value("")
            ->zero_tokens()
            ->notifier(
                [&](const std::string&) {
                    cfg.use_console(true);
                }),
         "override any logging options defined in configuration files and "
         "send all daemon output to the console"
        )

        // print the daemon version
        ("version,v",
         bpo::value<std::string>()
            ->implicit_value("")
            ->zero_tokens(),
         "print version string")

        // print help
        ("help,h",
         bpo::value<std::string>()
            ->implicit_value("")
            ->zero_tokens(),
         "produce help message")
    ;

    // parse the command line
    bpo::variables_map vm;

    try {
        bpo::store(bpo::parse_command_line(argc, argv, opt_desc), vm);

        // the --help and --version arguments are special, since we want 
        // to process them even if the global configuration file doesn't exist
        if(vm.count("help")) {
            print_help(cfg.progname(), opt_desc);
            return EXIT_SUCCESS;
        }

        if(vm.count("version")) {
            print_version(cfg.progname());
            return EXIT_SUCCESS;
        }

        if(!bfs::exists(cfg.config_file())) {
            fmt::print(stderr, "Failed to access daemon configuration file {}\n",
                    cfg.config_file());
            return EXIT_FAILURE;
        }

        try {
            cfg.load_from_file(cfg.config_file());
        }
        catch(const std::exception& ex) {
            fmt::print(stderr, "Failed reading daemon configuration file:\n"
                            "    {}\n", ex.what());
            return EXIT_FAILURE;
        }

        // calling notify() here basically invokes all define notifiers, thus 
        // overridingany configuration loaded from the global configuration file
        // with its command-line counterparts if provided (for those options where
        // this is available)
        bpo::notify(vm);
    }
    catch(const bpo::error& ex) {
        fmt::print(stderr, "ERROR: {}\n\n", ex.what());
        return EXIT_FAILURE;
    }

    try {
        norns::urd daemon;
        daemon.configure(cfg);
        return daemon.run();
    }
    catch(const std::exception& ex) {
        fmt::print(stderr, "An unhandled exception reached the top of main(), "
                           "{} will exit:\n  what():  {}\n", 
                           cfg.progname(), ex.what());
        return EXIT_FAILURE;
    }
}
