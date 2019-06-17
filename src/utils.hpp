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

#ifndef NORNS_UTILS_HPP
#define NORNS_UTILS_HPP

#include <boost/version.hpp>
#include <boost/filesystem.hpp>
#include <system_error>
#include <sstream>
#include <iomanip>
#include <cstdint>

#include "common.hpp"
#include "utils/tar-archive.hpp"
#include "utils/temporary-file.hpp"

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

boost::filesystem::path
remove_leading_separator(const boost::filesystem::path& pathname);

} // namespace utils
} // namespace norns

#if BOOST_VERSION <= 106000 // 1.6.0

#include <boost/filesystem.hpp>

namespace boost { namespace filesystem {

path relative(path from_path, path to_path);

}} // namespace boost::filesystem

#endif

#endif // NORNS_UTILS_HPP
