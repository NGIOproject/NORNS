/*************************************************************************
 * (C) Copyright 2016-2017 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 *                                                                       *
 * This file is part of the Echo Filesystem NG.                          *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU Lesser General Public            *
 * License as published by the Free Software Foundation; either          *
 * version 3 of the License, or (at your option) any later version.      *
 *                                                                       *
 * The Echo Filesystem NG is distributed in the hope that it will        *
 * be useful, but WITHOUT ANY WARRANTY; without even the implied         *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR               *
 * PURPOSE.  See the GNU Lesser General Public License for more          *
 * details.                                                              *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with Echo Filesystem NG; if not, write to the Free      *
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.    *
 *                                                                       *
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
