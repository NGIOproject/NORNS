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

#include <atomic>
#include <iostream>
#include "mpi-helpers.hpp"
#include "test-env.hpp"

namespace test_data {

struct {
    // define input names
    const bfs::path src_file_at_root = "/file0";
    const bfs::path src_file_at_subdir = "/a/b/c/d/file0";
    const bfs::path src_invalid_file = "/a/b/c/d/does_not_exist_file0";
    const bfs::path src_invalid_dir = "/a/b/c/d/does_not_exist_dir0";
    const bfs::path src_subdir0 = "/input_dir0";
    const bfs::path src_subdir1 = "/input_dir0/a/b/c/input_dir1";
    const bfs::path src_empty_dir = "/empty_dir0";

    const bfs::path src_noperms_file0 = "/noperms_file0";
    const bfs::path src_noperms_file1 = "/noperms/a/b/c/d/noperms_file0"; // parents accessible
    const bfs::path src_noperms_file2 = "/noperms/noperms_subdir0/file0"; // parents non-accessible
    const bfs::path src_noperms_subdir0 = "/noperms_subdir0"; // subdir non-accessible
    const bfs::path src_noperms_subdir1 = "/noperms/a/b/c/d/noperms_subdir1"; // child subdir non-accessible
    const bfs::path src_noperms_subdir2 = "/noperms/noperms_subdir2/a"; // parent subdir non-accessible

    const bfs::path src_symlink_at_root0 = "/symlink0";
    const bfs::path src_symlink_at_root1 = "/symlink1";
    const bfs::path src_symlink_at_root2 = "/symlink2";
    const bfs::path src_symlink_at_subdir0 = "/foo/bar/baz/symlink0";
    const bfs::path src_symlink_at_subdir1 = "/foo/bar/baz/symlink1";
    const bfs::path src_symlink_at_subdir2 = "/foo/bar/baz/symlink2";

    const bfs::path dst_root            = "/";
    const bfs::path dst_subdir0         = "/output_dir0";
    const bfs::path dst_subdir1         = "/output_dir1";
    const bfs::path dst_file_at_root0   = "/file0"; // same basename
    const bfs::path dst_file_at_root1   = "/file1"; // different basename
    const bfs::path dst_file_at_subdir0 = "/a/b/c/d/file0"; // same fullname
    const bfs::path dst_file_at_subdir1 = "/a/b/c/d/file1"; // same parents, different basename
    const bfs::path dst_file_at_subdir2 = "/e/f/g/h/i/file0"; // different parents, same basename
    const bfs::path dst_file_at_subdir3 = "/e/f/g/h/i/file1"; // different fullname
} context;

}

namespace server {

std::atomic<bool> shutdown(false);

void
install_signal_handlers() {

    for(auto signum : { SIGINT, SIGILL, SIGFPE, 
                        SIGSEGV, SIGTERM, SIGABRT }) {
        struct sigaction sa;
        sa.sa_handler = [](int signum) -> void {
            server::shutdown = true;
        };

        if(::sigaction(signum, &sa, NULL) != 0) {
            throw std::runtime_error("Failed to set signal handler");
        }
    }
}

}

int
main(int argc, char* argv[]) {

    using test_data::context;

    server::install_signal_handlers();

    std::string hostfile(argv[1]);

    mpi::read_hosts(hostfile);

    mpi::initialize(&argc, &argv);

    const std::string bind_address(
        mpi::test_hosts.at(mpi::get_rank() + 1).first + ":" +
        std::to_string(mpi::test_hosts.at(mpi::get_rank() + 1).second));

    if(::setenv("TEST_FORCE_BIND_ADDRESS", bind_address.c_str(), 1)) {
        throw std::runtime_error(
            "Failed to set TEST_FORCE_BIND_ADDRESS env var");
    }

    std::cout << "Server booting (" << bind_address << ")\n";

    test_env env;

    std::string nsid("server" + std::to_string(mpi::get_rank()));
    bfs::path mountdir;

    // create namespaces
    std::tie(std::ignore, mountdir) = 
        env.create_namespace(nsid, "mnt/" + nsid, 16384);

    // create required output directories
    env.add_to_namespace(nsid, context.dst_subdir1);

    std::cout << "Server ready\n";

    while(!server::shutdown) {
        ::usleep(1000);
    }

    std::cout << "Shutting down\n";

    env.notify_success();
}
