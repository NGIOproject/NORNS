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

#include <algorithm>
#include <iterator>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "utils.hpp"
#include "compare-files.hpp"

namespace bfs = boost::filesystem;

bool compare_files(const bfs::path& filename1, const bfs::path& filename2) {

    bfs::ifstream file1(filename1, std::ifstream::ate | std::ifstream::binary);

    if(!file1) {
        throw std::invalid_argument("could not open " + filename1.string());
    }

    bfs::ifstream file2(filename2, std::ifstream::ate | std::ifstream::binary);

    if(!file2) {
        throw std::invalid_argument("could not open " + filename2.string());
    }

    if(file1.tellg() != file2.tellg()) {
        return false;
    }

    file1.seekg(0);
    file2.seekg(0);

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin2(file2);
    std::istreambuf_iterator<char> end;

    return range_equal(begin1, end, begin2, end);
}

bool compare_directories(const bfs::path& dirname1, const bfs::path& dirname2) {

    bfs::recursive_directory_iterator it(dirname1);
    bfs::recursive_directory_iterator end;

    for(; it != end; ++it) {
        boost::system::error_code ec;
        const bfs::path pdst = bfs::canonical(dirname2 / bfs::relative(dirname1, *it), ec);

        if(ec) {
            return false;
        }

        if(bfs::is_directory(*it) && bfs::is_directory(pdst)) {
            continue;
        }
        else if(bfs::is_directory(pdst)){
            return false;
        }

        if(!compare_files(*it, pdst)) {
            return false;
        }
    }

    return true;
}

