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
#include "compare-files.hpp"
#include "test-env.hpp"
#include "catch.hpp"

namespace {

constexpr static const std::size_t tar_header_size = 
    norns::utils::tar::TAR_BLOCK_SIZE;
constexpr static const std::size_t tar_record_size = 
    norns::utils::tar::TAR_BLOCK_SIZE;
constexpr static const std::size_t tar_eof_size = 
    2*norns::utils::tar::TAR_BLOCK_SIZE;

constexpr std::size_t
tar_aligned_size(std::size_t sz) {
    return sz % tar_record_size != 0 ?
            ((sz & ~(tar_record_size - 1)) + tar_record_size) : 
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

std::string 
exec(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    int status;

    std::cout << "exec(\"" << cmd << "\")";

    auto deleter = [&status](FILE* stream) {
        status = ::pclose(stream);
    };

    std::unique_ptr<FILE, decltype(deleter)> pipe(
            ::popen(cmd.c_str(), "r"), deleter);
            
    if(!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while(::fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

std::error_code
extract(const bfs::path archive_path,
        const bfs::path parent_dir) {

    std::error_code ec;

    if(!bfs::exists(parent_dir)) {
        boost::system::error_code bec;
        bfs::create_directories(parent_dir, bec);

        if(bec) {
            ec.assign(bec.value(), std::generic_category());
            return ec;
        }
    }

    std::string cmd("tar xf " + archive_path.string() + " -C " +
                    parent_dir.string());
    auto res = ::exec(cmd);

    std::cout << res << "\n";

    return ec;
}

}

SCENARIO("tar creation", "[utils::tar::tar(filename, tar::create)]") {

    GIVEN("an invalid output path") {
        test_env env;
        using norns::utils::tar;

        bfs::path p = env.basedir();

        WHEN("creating a tar archive") {
            std::error_code ec;
            tar t(p, tar::create, ec);

            THEN("constructor fails") {
                REQUIRE(ec);
            }
        }

        env.notify_success();
    }

    GIVEN("a valid output path") {
        test_env env;
        using norns::utils::tar;

        bfs::path p = env.basedir() / "archive.tar";

        WHEN("creating a tar archive") {
            std::error_code ec;
            tar t(p, tar::create, ec);

            THEN("constructor succeeds") {
                REQUIRE(!ec);
                REQUIRE(bfs::exists(p));
            }
        }

        env.notify_success();
    }
}

SCENARIO("appending files to a TAR archive", 
         "[utils::tar::add_file(path, alias, error_code)]") {

    GIVEN("an invalid tar instance") {
        test_env env;
        using norns::utils::tar;

        std::error_code ec;
        tar t("", tar::create, ec);
        REQUIRE(ec);

        WHEN("trying to add a file") {

            bfs::path file = env.create_file("/regular_file", env.basedir(), 0);

            THEN("add_file() returns EINVAL") {
                t.add_file(file, bfs::relative(file, env.basedir()), ec);
                REQUIRE(ec);
                REQUIRE(ec.value() == EINVAL);
            }
        }

        env.notify_success();
    }

    GIVEN("a valid tar instance") {
        test_env env;
        using norns::utils::tar;

        std::array<std::string, 6> filenames = {
            "/regular_file0", "/regular_file1", "/regular_file2",
            "/regular_file3", "/regular_file4", "/regular_file5",
        };

        std::array<std::size_t, 6> sizes = { 
            0, 10000, 20000, 2134513, 42, 50124887
        };

        std::vector<bfs::path> file_paths;

        for(std::size_t i = 0; i < filenames.size(); ++i) {
            file_paths.emplace_back(
                    env.create_file(filenames[i], env.basedir(), sizes[i]));
        }

        bfs::path p = env.basedir() / "archive.tar";

        std::error_code ec;
        tar t(p, tar::create, ec);
        REQUIRE(!ec);

        WHEN("adding an empty file") {

            t.add_file(file_paths[0], 
                    bfs::relative(file_paths[0], env.basedir()), ec);

            THEN("add_file() succeeds") {
                REQUIRE(!ec);

                AND_THEN("the archive contains the added file") {

                    t.release(); // make sure data has been written to archive

                    const bfs::path parent_dir(env.basedir() / "tmp");
                    ec = ::extract(t.path(), parent_dir);

                    REQUIRE(!ec);
                    REQUIRE(bfs::exists(parent_dir / filenames[0]));
                    REQUIRE(compare_files(
                                file_paths[0], parent_dir / filenames[0]));
                }
            }
        }

        WHEN("adding a non-empty file") {
            t.add_file(file_paths[1], 
                        bfs::relative(file_paths[1], env.basedir()), ec);

            THEN("add_file() succeeds") {
                REQUIRE(!ec);

                AND_THEN("the archive contains the added file") {

                    t.release(); // make sure data has been written to archive

                    const bfs::path parent_dir(env.basedir() / "tmp");
                    ec = ::extract(t.path(), parent_dir);

                    REQUIRE(!ec);
                    REQUIRE(bfs::exists(parent_dir / filenames[1]));
                    REQUIRE(compare_files(
                                file_paths[1], parent_dir / filenames[1]));
                }
            }
        }

        WHEN("adding several files") {

            std::vector<std::error_code> ecs;

            for(std::size_t i = 0; i < file_paths.size(); ++i) {
                t.add_file(file_paths[i], 
                           bfs::relative(file_paths[i], env.basedir()), ec);

                ecs.emplace_back(ec);

            }

            THEN("add_file() succeeds") {

                for(const auto err : ecs) {
                    REQUIRE(!err);
                }

                AND_THEN("the archive contains all added files") {

                    t.release(); // make sure data has been written to archive

                    const bfs::path parent_dir(env.basedir() / "tmp");
                    ec = ::extract(t.path(), parent_dir);
                    REQUIRE(!ec);

                    for(std::size_t i = 0; i < file_paths.size(); ++i) {
                        REQUIRE(bfs::exists(parent_dir / filenames[i]));
                        REQUIRE(compare_files(
                                    file_paths[i], parent_dir / filenames[i]));
                    }
                }
            }
        }

        env.notify_success();
    }

}

SCENARIO("appending directories to a TAR archive",
         "[utils::tar::add_directory(path, alias, error_code)]") {

    GIVEN("an invalid tar instance") {
        test_env env;
        using norns::utils::tar;

        std::error_code ec;
        tar t("", tar::create, ec);
        REQUIRE(ec);

        WHEN("trying to add a directory") {

            bfs::path subdir = env.create_directory("/subdir", env.basedir());

            THEN("add_directory() returns EINVAL") {
                t.add_directory(subdir, 
                        bfs::relative(subdir, env.basedir()), ec);
                REQUIRE(ec);
                REQUIRE(ec.value() == EINVAL);
            }
        }

        env.notify_success();
    }

    GIVEN("a valid tar instance") {
        test_env env;
        using norns::utils::tar;


        std::array<std::string, 6> filenames = {
            "/regular_file0", "/regular_file1", "/regular_file2",
            "/regular_file3", "/regular_file4", "/regular_file5",
        };

        std::array<std::size_t, 6> sizes = { 
            0, 10000, 20000, 2134513, 42, 5012488//7
        };

        std::vector<bfs::path> file_paths;

        bfs::path subdir = 
            env.create_directory("/subdir", env.basedir());

        for(std::size_t i = 0; i < filenames.size(); ++i) {
            file_paths.emplace_back(
                    env.create_file(filenames[i], subdir, sizes[i]));
        }

        bfs::path p = env.basedir() / "archive.tar";

        std::error_code ec;
        tar t(p, tar::create, ec);
        REQUIRE(!ec);

        WHEN("adding an empty directory") {

            bfs::path empty_subdir = 
                env.create_directory("/empty_subdir", env.basedir());

            t.add_directory(empty_subdir, 
                    bfs::relative(empty_subdir, env.basedir()), ec);

            THEN("add_directory() succeeds") {
                REQUIRE(!ec);

                AND_THEN("the archive contains the added directory") {

                    t.release(); // make sure data has been written to archive

                    const bfs::path parent_dir(env.basedir() / "tmp");
                    ec = ::extract(t.path(), parent_dir);

                    REQUIRE(!ec);
                    REQUIRE(bfs::exists(parent_dir / "/empty_subdir"));
                    REQUIRE(compare_directories(
                                empty_subdir, parent_dir / "/empty_subdir"));
                }
            }
        }

        WHEN("adding a non-empty directory containing only files and "
             "defining a 1-level alias (e.g. subdir0/ -> subdir1/ ) ") {

            t.add_directory(subdir, 
                        bfs::relative(subdir, env.basedir()), ec);

            THEN("add_directory() succeeds") {
                REQUIRE(!ec);

                AND_THEN("the archive contains the added directory") {

                    t.release(); // make sure data has been written to archive

                    const bfs::path parent_dir(env.basedir() / "tmp");
                    ec = ::extract(t.path(), parent_dir);

                    REQUIRE(!ec);
                    REQUIRE(bfs::exists(parent_dir / "/subdir"));

					AND_THEN("both directories are identical") {
						REQUIRE(compare_directories(
									subdir, parent_dir / "/subdir"));
					}
                }
            }
        }

        WHEN("adding a non-empty directory containing only files and defining "
             "a multi-level alias (e.g. subdir0/ -> subdir1/a/b/c/ )") {

            t.add_directory(subdir, 
                        bfs::relative(subdir, env.basedir()) / "a/b/c/", ec);

            THEN("add_directory() succeeds") {
                REQUIRE(!ec);

                AND_THEN("the archive contains the added directory") {

                    t.release(); // make sure data has been written to archive

                    const bfs::path parent_dir(env.basedir() / "tmp");
                    ec = ::extract(t.path(), parent_dir);

                    REQUIRE(!ec);
                    REQUIRE(bfs::exists(parent_dir / "/subdir"));
                    REQUIRE(bfs::exists(parent_dir / "/subdir/a/"));
                    REQUIRE(bfs::exists(parent_dir / "/subdir/a/b/"));
                    REQUIRE(bfs::exists(parent_dir / "/subdir/a/b/c/"));

					AND_THEN("contents are identical") {
						REQUIRE(compare_directories(
									subdir, parent_dir / "/subdir/a/b/c"));
					}
                }
            }
        }

        WHEN("adding a non-empty directory containing only files and "
             "defining an empty alias") {

            t.add_directory(subdir, "", ec);

            THEN("add_directory() succeeds") {
                REQUIRE(!ec);

                AND_THEN("the archive does not contain the added directory") {

                    t.release(); // make sure data has been written to archive

                    const bfs::path parent_dir(env.basedir() / "tmp");
                    ec = ::extract(t.path(), parent_dir);

                    REQUIRE(!ec);
                    REQUIRE(!bfs::exists(parent_dir / "/subdir"));

					AND_THEN("contents are identical") {
						REQUIRE(compare_directories(subdir, parent_dir));
					}
                }
            }
        }

        WHEN("adding a non-empty directory containing only files and "
             "defining '/' as an alias") {

            t.add_directory(subdir, "/", ec);

            THEN("add_directory() succeeds") {
                REQUIRE(!ec);

                AND_THEN("the archive does not contain the added directory") {

                    t.release(); // make sure data has been written to archive

                    const bfs::path parent_dir(env.basedir() / "tmp");
                    ec = ::extract(t.path(), parent_dir);

                    REQUIRE(!ec);
                    REQUIRE(!bfs::exists(parent_dir / "/subdir"));

					AND_THEN("contents are identical") {
						REQUIRE(compare_directories(subdir, parent_dir));
					}
                }
            }
        }

        WHEN("adding a non-empty directory containing files and subdirs, and "
             "defining a 1-level alias (e.g. subdir0/ -> subdir1/ ) ") {

			size_generator gen(rng{42}, distribution{0, 42});

            bfs::path complex_subdir =
                create_hierarchy(env, gen, "/complex_subdir", env.basedir(), 
                                 2, 5, 5);

            t.add_directory(complex_subdir, 
                        bfs::relative(complex_subdir, env.basedir()), ec);

            THEN("add_directory() succeeds") {
                REQUIRE(!ec);

                AND_THEN("the archive contains the added directory") {

                    t.release(); // make sure data has been written to archive

                    const bfs::path parent_dir(env.basedir() / "tmp");
                    ec = ::extract(t.path(), parent_dir);

                    REQUIRE(!ec);
                    REQUIRE(bfs::exists(parent_dir / "/complex_subdir"));

					AND_THEN("both directories are identical") {
						REQUIRE(compare_directories(
									complex_subdir, 
                                    parent_dir / "/complex_subdir"));
					}
                }
            }
        }

        WHEN("adding a non-empty directory containing files and subdirs, and "
             "defining a multi-level alias (e.g. subdir0/ -> "
             "subdir1/a/b/c/ )") {

			size_generator gen(rng{42}, distribution{0, 42});

            bfs::path complex_subdir =
                create_hierarchy(env, gen, "/complex_subdir", env.basedir(), 
                                 2, 5, 5);

            t.add_directory(
                complex_subdir,
                bfs::relative(complex_subdir, env.basedir()) / "a/b/c/", ec);

            THEN("add_directory() succeeds") {
                REQUIRE(!ec);

                AND_THEN("the archive contains the added directory") {

                    t.release(); // make sure data has been written to archive

                    const bfs::path parent_dir(env.basedir() / "tmp");
                    ec = ::extract(t.path(), parent_dir);

                    REQUIRE(!ec);
                    REQUIRE(bfs::exists(parent_dir / "/complex_subdir"));
                    REQUIRE(bfs::exists(parent_dir / "/complex_subdir/a/"));
                    REQUIRE(bfs::exists(parent_dir / "/complex_subdir/a/b/"));
                    REQUIRE(bfs::exists(parent_dir / "/complex_subdir/a/b/c/"));

					AND_THEN("both directories are identical") {
						REQUIRE(compare_directories(
									complex_subdir, 
                                    parent_dir / "/complex_subdir/a/b/c"));
					}
                }
            }
        }

        env.notify_success();
    }
}

SCENARIO("extracting from a tar archive", 
         "[utils::tar::extract(destination_path, error_code)]") {

    GIVEN("an invalid tar instance") {
        test_env env;
        using norns::utils::tar;

        std::error_code ec;
        tar t("", tar::open, ec);
        REQUIRE(ec);

        WHEN("trying to extract the archive") {

			std::error_code ec;
			bfs::path tmpdir = env.create_directory("/tmp", env.basedir());
			t.extract(tmpdir, ec);

            THEN("extract() returns EINVAL") {
                REQUIRE(ec);
                REQUIRE(ec.value() == EINVAL);
            }
        }

        env.notify_success();
    }

    GIVEN("a valid tar instance") {
        test_env env;
        using norns::utils::tar;

		bfs::path tmp_dir = env.create_directory("/tmp", env.basedir());
		size_generator gen(rng(42), distribution(0, 42));
		bfs::path complex_subdir = 
			create_hierarchy(env, gen, "/complex_subdir", env.basedir(), 
                             2, 5, 5);
        bfs::path p = env.basedir() / "archive.tar";

		{
			std::error_code ec;
			tar t(p, tar::create, ec);
			REQUIRE(!ec);
			t.add_directory(complex_subdir, "", ec);
			REQUIRE(!ec);
		}

        WHEN("trying to extract the archive to \"\"") {

            std::error_code ec;
            tar t(p, tar::open, ec);
            REQUIRE(!ec);

			t.extract("", ec);

            THEN("extract() returns EINVAL") {
                REQUIRE(ec);
                REQUIRE(ec.value() == EINVAL);
            }
        }

        env.notify_success();
    }

    GIVEN("a valid tar instance") {
        test_env env;
        using norns::utils::tar;

		bfs::path tmp_dir = env.create_directory("/tmp", env.basedir());
		size_generator gen(rng(42), distribution(0, 42));
		bfs::path complex_subdir = 
			create_hierarchy(env, gen, "/complex_subdir", env.basedir(), 
                             2, 5, 5);
        bfs::path p = env.basedir() / "archive.tar";

		{
			std::error_code ec;
			tar t(p, tar::create, ec);
			REQUIRE(!ec);
			t.add_directory(complex_subdir, "", ec);
			REQUIRE(!ec);
		}

        WHEN("trying to extract the archive to a path leading to a file") {

            std::error_code ec;
            tar t(p, tar::open, ec);
            REQUIRE(!ec);

			t.extract(p, ec);

            THEN("extract() returns EINVAL") {
                REQUIRE(ec);
                REQUIRE(ec.value() == EINVAL);
            }
        }

        env.notify_success();
    }

    GIVEN("a valid tar instance") {
        test_env env;
        using norns::utils::tar;

		bfs::path tmp_dir = env.create_directory("/tmp", env.basedir());
        bfs::path file = env.create_file("/fake.tar", env.basedir(), 0);

        WHEN("trying to extract from something that looks like an archive ") {

            std::error_code ec;
            tar t(file, tar::open, ec);

            THEN("constructor() fails") {
                REQUIRE(ec);
            }
        }

        env.notify_success();
    }

    GIVEN("a valid tar instance and an archive with alias \"\"") {
        test_env env;
        using norns::utils::tar;

		bfs::path tmp_dir = env.create_directory("/tmp", env.basedir());
		size_generator gen(rng(42), distribution(0, 42));
		bfs::path complex_subdir = 
			create_hierarchy(env, gen, "/complex_subdir", env.basedir(), 
                             2, 5, 5);
        bfs::path p = env.basedir() / "archive.tar";

        std::error_code ec;
		{
			tar t(p, tar::create, ec);
			REQUIRE(!ec);

			t.add_directory(complex_subdir, "", ec);
			REQUIRE(!ec);
		}

        WHEN("trying to extract the archive to a path leading to a directory") {

			tar t(p, tar::open, ec);
			REQUIRE(!ec);

            THEN("extract() succeeds") {
                t.extract(tmp_dir, ec);
                REQUIRE(!ec);

                AND_THEN("the specified directory contains the archive "
                         "contents and they are identical to the original") {
                    REQUIRE(compare_directories(complex_subdir, tmp_dir));
                }
            }
        }

        env.notify_success();
    }

    GIVEN("a valid tar instance and an archive with a 1-level alias") {
        test_env env;
        using norns::utils::tar;

		bfs::path tmp_dir = env.create_directory("/tmp", env.basedir());
		size_generator gen(rng(42), distribution(0, 42));
		bfs::path complex_subdir = 
			create_hierarchy(env, gen, "/complex_subdir", env.basedir(), 
                             2, 5, 5);
        bfs::path p = env.basedir() / "archive.tar";

        std::error_code ec;
		{
			tar t(p, tar::create, ec);
			REQUIRE(!ec);

			t.add_directory(complex_subdir, "/alias_subdir", ec);
			REQUIRE(!ec);
		}

        WHEN("trying to extract the archive to a path leading to a directory") {

			tar t(p, tar::open, ec);
			REQUIRE(!ec);

            THEN("extract() succeeds") {
                t.extract(tmp_dir, ec);
                REQUIRE(!ec);

                AND_THEN("the specified directory contains the archive "
                         "contents and they are identical to the original") {
                    REQUIRE(bfs::exists(tmp_dir / "/alias_subdir"));
                    REQUIRE(compare_directories(
                                complex_subdir, tmp_dir / "/alias_subdir"));
                }
            }
        }

        env.notify_success();
    }

    GIVEN("a valid tar instance and an archive with a multi-level alias") {
        test_env env;
        using norns::utils::tar;

		bfs::path tmp_dir = env.create_directory("/tmp", env.basedir());
		size_generator gen(rng(42), distribution(0, 42));
		bfs::path complex_subdir = 
			create_hierarchy(env, gen, "/complex_subdir", env.basedir(), 
                             2, 5, 5);
        bfs::path p = env.basedir() / "archive.tar";

        std::error_code ec;
		{
			tar t(p, tar::create, ec);
			REQUIRE(!ec);

			t.add_directory(complex_subdir, "/alias_subdir/a/b/c", ec);
			REQUIRE(!ec);
		}

        WHEN("trying to extract the archive to a path leading to a directory") {

			tar t(p, tar::open, ec);
			REQUIRE(!ec);

            THEN("extract() succeeds") {
                t.extract(tmp_dir, ec);
                REQUIRE(!ec);

                AND_THEN("the specified directory contains the archive "
                         "contents and they are identical to the original") {
                    REQUIRE(bfs::exists(tmp_dir / "/alias_subdir/a/b/c"));
                    REQUIRE(compare_directories(
                                complex_subdir, 
                                tmp_dir / "/alias_subdir/a/b/c"));
                }
            }
        }

        env.notify_success();
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

    GIVEN("a path to a regular file of tar_record_size bytes") {

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

    GIVEN("a path to a regular file of 2*tar_record_size bytes") {

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

                    t.add_directory(subdir, 
                                    bfs::relative(subdir, env.basedir()), ec);
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

    GIVEN("a path to a directory hierarchy containing empty files and an "
          "empty alias") {

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

                t.add_directory(subdir, "", ec);
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
