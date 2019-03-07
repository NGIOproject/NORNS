/*************************************************************************
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
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

#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>
#include "utils.hpp"
#include "test-env.hpp"
#include "catch.hpp"

namespace {

static constexpr std::size_t tar_header_size = T_BLOCKSIZE;
static constexpr std::size_t tar_eof_size = 2*T_BLOCKSIZE;

constexpr std::size_t
tar_aligned_size(std::size_t sz) {
    return sz % T_BLOCKSIZE != 0 ?
            ((sz & ~(T_BLOCKSIZE - 1)) + T_BLOCKSIZE) : 
            sz;
}


using rng = boost::mt19937;
using distribution = boost::uniform_int<>;
using size_generator = 
    boost::variate_generator<rng, distribution>;

bfs::path
create_hierarchy(test_env& env,
                 size_generator& size_gen,
                 const bfs::path& subdir,
                 const bfs::path& parent,
                 std::size_t subdirs_per_level,
                 std::size_t files_per_subdir,
                 std::size_t levels) {

    if(levels == 0) {
        return {};
    }

    bfs::path self = env.create_directory(subdir, parent);

    for(std::size_t i = 0; i < files_per_subdir; ++i) {
        std::string fname = "/regular_file" + std::to_string(i);
        auto p = env.create_file(fname, self, size_gen());
    }

    if(levels == 1) {
        return self;
    }

    for(std::size_t i = 0; i < subdirs_per_level; ++i) {
        std::string dname = "/subdir" + std::to_string(i);
        auto child = env.create_directory(dname, self);

        (void) create_hierarchy(env, size_gen, dname, self, subdirs_per_level, 
                                files_per_subdir, levels - 1);
    }

    return self;
}

}

SCENARIO("tar size estimation", "[utils::tar::estimate_packed_size()]") {

    GIVEN("a path to a regular file of 0 bytes") {

        test_env env;
        using norns::utils::tar;

        bfs::path file = env.create_file("/regular_file", env.basedir(), 0);

        THEN("estimated size follows formula") {
            std::error_code ec;
            std::size_t psize = tar::estimate_size_once_packed(file, ec);

            REQUIRE(psize == tar_header_size + 
                             tar_eof_size);

            AND_THEN("size of generated TAR archive matches estimated one") {

                bfs::path file_tar = file;
                file_tar.replace_extension(".tar");

                {
                    tar t(file_tar, tar::create, ec);
                    REQUIRE(!ec);

                    t.add_file(file, bfs::relative(file, env.basedir()), ec);
                    REQUIRE(!ec);
                }

                // this check needs to be done after tar::~tar() has been
                // called, since it appends the EOF blocks
                REQUIRE(psize == bfs::file_size(file_tar));
            }
        }

        env.notify_success();
    }

    GIVEN("a path to a regular file of T_BLOCKSIZE bytes") {

        test_env env;
        using norns::utils::tar;

        bfs::path file = 
            env.create_file("/regular_file", env.basedir(), tar_header_size);

        THEN("estimated size follows formula") {
            std::error_code ec;
            std::size_t psize = tar::estimate_size_once_packed(file, ec);

            REQUIRE(psize == tar_header_size + 
                             tar_header_size + 
                             tar_eof_size);

            AND_THEN("size of generated TAR archive matches estimated one") {

                bfs::path file_tar = file;
                file_tar.replace_extension(".tar");

                {
                    tar t(file_tar, tar::create, ec);
                    REQUIRE(!ec);

                    t.add_file(file, bfs::relative(file, env.basedir()), ec);
                    REQUIRE(!ec);
                }

                // this check needs to be done after tar::~tar() has been
                // called, since it appends the EOF blocks
                REQUIRE(psize == bfs::file_size(file_tar));
            }
        }

        env.notify_success();
    }

    GIVEN("a path to a regular file of 2*T_BLOCKSIZE bytes") {

        test_env env;
        using norns::utils::tar;

        bfs::path file = 
            env.create_file("/regular_file", env.basedir(), 2*tar_header_size);

        THEN("estimated size follows formula") {
            std::error_code ec;
            std::size_t psize = tar::estimate_size_once_packed(file, ec);

            REQUIRE(psize == tar_header_size + 
                             2*tar_header_size + 
                             tar_eof_size);

            AND_THEN("size of generated TAR archive matches estimated one") {

                bfs::path file_tar = file;
                file_tar.replace_extension(".tar");

                {
                    tar t(file_tar, tar::create, ec);
                    REQUIRE(!ec);

                    t.add_file(file, bfs::relative(file, env.basedir()), ec);
                    REQUIRE(!ec);
                }

                // this check needs to be done after tar::~tar() has been
                // called, since it appends the EOF blocks
                REQUIRE(psize == bfs::file_size(file_tar));
            }
        }

        env.notify_success();
    }

    GIVEN("a path to a regular file of 10000 bytes") {

        test_env env;
        using norns::utils::tar;

        bfs::path file = 
            env.create_file("/regular_file", env.basedir(), 10000);

        THEN("estimated size follows formula") {
            std::error_code ec;
            std::size_t psize = tar::estimate_size_once_packed(file, ec);

            REQUIRE(psize == tar_header_size + 
                             tar_aligned_size(10000) + 
                             tar_eof_size);

            AND_THEN("size of generated TAR archive matches estimated one") {

                bfs::path file_tar = file;
                file_tar.replace_extension(".tar");

                {
                    tar t(file_tar, tar::create, ec);
                    REQUIRE(!ec);

                    t.add_directory(file, 
                                    bfs::relative(file, env.basedir()), ec);
                    REQUIRE(!ec);
                }

                // this check needs to be done after tar::~tar() has been
                // called, since it appends the EOF blocks
                REQUIRE(psize == bfs::file_size(file_tar));
            }
        }

        env.notify_success();
    }


    GIVEN("a path to an empty directory") {

        test_env env;
        using norns::utils::tar;

        bfs::path subdir = env.create_directory("/subdir", env.basedir());

        THEN("estimated size follows formula") {
            std::error_code ec;
            std::size_t psize = tar::estimate_size_once_packed(subdir, ec);

            REQUIRE(psize == tar_header_size + 
                             tar_eof_size);

            AND_THEN("size of generated TAR archive matches estimated one") {

                bfs::path subdir_tar = subdir;
                subdir_tar.replace_extension(".tar");

                {
                    tar t(subdir_tar, tar::create, ec);
                    REQUIRE(!ec);

                    t.add_file(subdir, bfs::relative(subdir, env.basedir()), ec);
                    REQUIRE(!ec);
                }

                // this check needs to be done after tar::~tar() has been
                // called, since it appends the EOF blocks
                REQUIRE(psize == bfs::file_size(subdir_tar));
            }
        }

        env.notify_success();
    }

    GIVEN("a path to a directory hierarchy containing empty files") {

        test_env env;
        using norns::utils::tar;

        size_generator gen(rng{42}, distribution{0, 0});

        bfs::path subdir = 
            create_hierarchy(env, gen, "/subdir", env.basedir(), 5, 5, 5);

        THEN("size of generated TAR archive matches estimated one") {

            std::error_code ec;
            std::size_t psize = tar::estimate_size_once_packed(subdir, ec);

            bfs::path subdir_tar = subdir;
            subdir_tar.replace_extension(".tar");

            {
                tar t(subdir_tar, tar::create, ec);
                REQUIRE(!ec);

                t.add_directory(subdir, 
                                bfs::relative(subdir, env.basedir()), ec);
                REQUIRE(!ec);
            }

            // this check needs to be done after tar::~tar() has been
            // called, since it appends the EOF blocks
            REQUIRE(psize == bfs::file_size(subdir_tar));
        }

        env.notify_success();
    }

    GIVEN("a path to a directory hierarchy containing arbitrary files") {

        test_env env;
        using norns::utils::tar;

        size_generator gen(rng{42}, distribution{100, 42*1024});

        bfs::path subdir = 
            create_hierarchy(env, gen, "/subdir", env.basedir(), 5, 5, 5);

        THEN("size of generated TAR archive matches estimated one") {

            std::error_code ec;
            std::size_t psize = tar::estimate_size_once_packed(subdir, ec);

            bfs::path subdir_tar = subdir;
            subdir_tar.replace_extension(".tar");

            {
                tar t(subdir_tar, tar::create, ec);
                REQUIRE(!ec);

                t.add_directory(subdir, 
                                bfs::relative(subdir, env.basedir()), ec);
                REQUIRE(!ec);
            }

            // this check needs to be done after tar::~tar() has been
            // called, since it appends the EOF blocks
            REQUIRE(psize == bfs::file_size(subdir_tar));
        }

        env.notify_success();
    }

}
