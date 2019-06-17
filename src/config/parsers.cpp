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

#include <cstdint>
#include <string>
#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "utils.hpp"
#include "parsers.hpp"

namespace bfs = boost::filesystem;

namespace norns {
namespace config {
namespace parsers {

bool parse_bool(const std::string& name, const std::string& value) {

    if(value == "1" || boost::algorithm::to_lower_copy(value) == "true") {
        return true;
    }

    if(value == "0" || boost::algorithm::to_lower_copy(value) == "false") {
        return false;
    }

    throw std::invalid_argument("Value provided for option '" + name + "' is not boolean");
}

uint32_t parse_number(const std::string& name, const std::string& value) {

    int32_t optval = 0;

    try {
        optval = std::stoi(value);
    } catch(...) {
        throw std::invalid_argument("Value provided for option '" + name + "' is not a number");
    }

    if(optval <= 0) {
        throw std::invalid_argument("Value provided for option '" + name + "' must be greater than zero");
    }

    return static_cast<uint32_t>(optval);
}

bfs::path parse_path(const std::string& name, const std::string& value) {

    (void) name;

    return bfs::path(value);
}

bfs::path parse_existing_path(const std::string& name, const std::string& value) {

    if(!bfs::exists(value)) {
        throw std::invalid_argument("Path '" + value + "' in option '" + name + "' does not exist");
    }

    return bfs::path(value);
}

uint64_t parse_capacity(const std::string& name, const std::string& value) {

    try {
        return utils::parse_size(value);
    }
    catch(const std::exception& e) {
        throw std::invalid_argument("Value provided in option '" + name + "' is invalid");
    }
}

} // namespace parsers
} // namespace config
} // namespace norns
