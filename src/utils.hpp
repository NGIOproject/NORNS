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

#include <boost/version.hpp>
#include <boost/filesystem.hpp>
#include <system_error>
#include <sstream>
#include <iomanip>
#include <cstdint>

#include "common.hpp"

namespace norns {
namespace utils {

uint64_t parse_size(const std::string& str);

template <typename T> 
std::string n2hexstr(T i, bool zero_pad=false) {
    std::stringstream ss;

    if(zero_pad) {
       ss << std::setfill('0') << std::setw(sizeof(T) << 1);
    }

    ss << std::showbase << std::hex << i;
    return ss.str();
}

boost::filesystem::path lexical_normalize(const boost::filesystem::path& pathname,
                                          bool as_directory=false);

boost::filesystem::path
remove_trailing_separator(const boost::filesystem::path& pathname);

} // namespace utils
} // namespace norns

#if BOOST_VERSION <= 106000 // 1.6.0

#include <boost/filesystem.hpp>

namespace boost { namespace filesystem {

path relative(path from_path, path to_path);

}} // namespace boost::filesystem

#endif
