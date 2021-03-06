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

#include <string>
#include <algorithm>
#include <locale>
#include <cmath>

#include "utils.hpp"
#include "norns.h"

namespace norns {
namespace utils {

uint64_t parse_size(const std::string& str){

    constexpr const uint64_t B_FACTOR = 1;
    constexpr const uint64_t KB_FACTOR = 1e3;
    constexpr const uint64_t KiB_FACTOR = (1 << 10);
    constexpr const uint64_t MB_FACTOR = 1e6;
    constexpr const uint64_t MiB_FACTOR = (1 << 20);
    constexpr const uint64_t GB_FACTOR = 1e9;
    constexpr const uint64_t GiB_FACTOR = (1 << 30);

    const std::pair<const std::string, const uint64_t> conversions[] = {
        {"GiB", GiB_FACTOR},
        {"GB",  GB_FACTOR},
        {"G",   GB_FACTOR},
        {"MiB", MiB_FACTOR},
        {"MB",  MB_FACTOR},
        {"M",   MB_FACTOR},
        {"KiB", KiB_FACTOR},
        {"KB",  KB_FACTOR},
        {"K",   KB_FACTOR},
        {"B",   B_FACTOR},
    };

    std::string scopy(str);

    /* remove whitespaces from the string */
    scopy.erase(std::remove_if(scopy.begin(), scopy.end(), 
                                [](char ch){
                                    return std::isspace<char>(ch, std::locale::classic()); 
                                }), 
                scopy.end() );

    /* determine the units */
    std::size_t pos = std::string::npos;
    uint64_t factor = B_FACTOR;

    for(const auto& c: conversions){
        const std::string& suffix = c.first;

        if((pos = scopy.find(suffix)) != std::string::npos){
            /* check that the candidate is using the suffix EXACTLY 
             * to prevent accepting something like "GBfoo" as a valid Gigabyte unit */
            if(suffix != scopy.substr(pos)){
                pos = std::string::npos;
                continue;
            }

            factor = c.second;
            break;
        }
    }

    /* this works as expected because if pos == std::string::npos, the standard
     * states that all characters until the end of the string should be included.*/
    std::string number_str = scopy.substr(0, pos);

    /* check if it's a valid number */
    if(number_str == "" || !std::all_of(number_str.begin(), number_str.end(), ::isdigit)){
        throw std::invalid_argument("Not a number");
    }

    double value = std::stod(number_str);

    return std::round(value*factor);
}

// lexically remove any ./ and ../ components from a provided pathname
// (adapted from boost::filesystem::path::lexically_normal())
boost::filesystem::path lexical_normalize(const boost::filesystem::path& pathname,
                                          bool as_directory) {

    using boost::filesystem::path;

    if(pathname.empty()) { 
        return path{};
    }

    if(pathname == "/") {
        return path{"/"};
    }

    path tmp{"/"};

    for(const auto& elem : pathname) {
        if(elem == "..") {
            // move back on '../'
            tmp = tmp.remove_filename();

            if(tmp.empty()) {
                tmp = "/";
            }
        }
        else if(elem != "." && elem != "/") {
            // There is a weird case in some versions of boost, where the elem is returned
            // incorrectly for paths with 2 leading slashes:
            //    Example: *path('//a/b/c').begin() returns '//a' instead of '/'
            // If this happens, we extract the trailing component
            if(elem.native().size() > 2 && 
                    ((elem.native())[0] == '/' && (elem.native())[1] == '/')) {
                tmp /= elem.native().substr(2);
            }
            else {
                tmp /= elem;
            }
        }
        // ignore './'
    }

    if(tmp != "/" && as_directory) {
        tmp /= "/";
    }

    return tmp;
}


// remove a trailing separator
// (adapted from boost::filesystem::path::remove_trailing_separator())
boost::filesystem::path
remove_trailing_separator(const boost::filesystem::path& pathname) {

    using boost::filesystem::path;

    std::string str{pathname.generic_string()};

    auto is_directory_separator = [](path::value_type c) {
        return c == '/';
    };

    if(!str.empty() && is_directory_separator(str[str.size() - 1])) {
        str.erase(str.size() - 1);
    }

    return path{str};
}

// remove a leading separator
// (adapted from boost::filesystem::path::remove_trailing_separator())
boost::filesystem::path
remove_leading_separator(const boost::filesystem::path& pathname) {

    using boost::filesystem::path;

    std::string str{pathname.generic_string()};

    auto is_directory_separator = [](path::value_type c) {
        return c == '/';
    };

    if(!str.empty() && is_directory_separator(str[0])) {
        str.erase(0, 1);
    }

    return path{str};
}

} // namespace utils
} // namespace norns

#if BOOST_VERSION <= 106000 // 1.6.0
namespace boost { 
namespace filesystem {

template <> 
path& path::append<path::iterator>(path::iterator begin, path::iterator end, const codecvt_type& cvt) {

    (void) cvt;

    for(; begin != end; ++begin)
        *this /= *begin;
    return *this;
}

/* Return path that 'p' made relative to 'base' */
path relative(path p, path base=boost::filesystem::current_path()) {

    path ret;
    base = absolute(base);
    p = absolute(p);

    path::const_iterator base_it(base.begin()); 
    path::const_iterator p_it(p.begin());

    // Find common base
    for(path::const_iterator to_end(p.end()), from_end(base.end()); 
        base_it != from_end && p_it != to_end && *base_it == *p_it; 
        ++base_it, ++p_it
       );

    // Navigate backwards in directory to reach previously found base
    for(path::const_iterator from_end(base.end()); base_it != from_end; ++base_it) {
        if((*base_it) != ".")
            ret /= "..";
    }

    // Now navigate down the directory branch
    ret.append(p_it, p.end(), path::codecvt());

    return ret;
}

} 
} // namespace boost::filesystem
#endif
