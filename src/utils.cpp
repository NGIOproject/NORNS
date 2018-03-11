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

#include <string>
#include <algorithm>
#include <locale>
#include "utils.hpp"
#include "norns.h"

namespace norns {
namespace utils {

uint64_t parse_size(const std::string& str){

    const uint64_t B_FACTOR = 1;
    const uint64_t KB_FACTOR = 1e3;
    const uint64_t KiB_FACTOR = (1 << 10);
    const uint64_t MB_FACTOR = 1e6;
    const uint64_t MiB_FACTOR = (1 << 20);
    const uint64_t GB_FACTOR = 1e9;
    const uint64_t GiB_FACTOR = (1 << 30);

    std::pair<std::string, uint64_t> conversions[] = {
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

std::string strerror(int error_code) {

    switch(error_code) {
        case NORNS_SUCCESS:
            return "NORNS_SUCCESS";
        case NORNS_ESNAFU:
            return "NORNS_ESNAFU";
        case NORNS_EBADARGS:
            return "NORNS_EBADARGS";
        case NORNS_EBADREQUEST:
            return "NORNS_EBADREQUEST";
        case NORNS_ENOMEM:
            return "NORNS_ENOMEM";
        case NORNS_ECONNFAILED:
            return "NORNS_ECONNFAILED";
        case NORNS_ERPCSENDFAILED:
            return "NORNS_ERPCSENDFAILED";
        case NORNS_ERPCRECVFAILED:
            return "NORNS_ERPCRECVFAILED";
        case NORNS_EJOBEXISTS:
            return "NORNS_EJOBEXISTS";
        case NORNS_ENOSUCHJOB:
            return "NORNS_ENOSUCHJOB";
        case NORNS_EPROCESSEXISTS:
            return "NORNS_EPROCESSEXISTS";
        case NORNS_ENOSUCHPROCESS:
            return "NORNS_ENOSUCHPROCESS";
        case NORNS_EBACKENDEXISTS:
            return "NORNS_EBACKENDEXISTS";
        case NORNS_ENOSUCHBACKEND:
            return "NORNS_ENOSUCHBACKEND";

        case NORNS_ENOTSUPPORTED:
            return "NORNS_ENOTSUPPORTED";

        default:
            return "UNKNOWN_ERROR";
    }
}

std::string to_string(uint32_t c) {

    switch(c) {
        case NORNS_IOTASK_COPY:
            return "DATA_COPY";
        case NORNS_IOTASK_MOVE:
            return "DATA_MOVE";
        case NORNS_BACKEND_NVML:
            return "NVML";
        case NORNS_BACKEND_LUSTRE:
            return "LUSTRE";
        case NORNS_BACKEND_PROCESS_MEMORY:
            return "PROCESS_MEMORY";
        case NORNS_BACKEND_ECHOFS:
            return "ECHOFS";
        case NORNS_BACKEND_POSIX_FILESYSTEM:
            return "POSIX_FILESYSTEM";
        default:
            return "UNKNOWN_FIELD";
    }
}

} // namespace utils
} // namespace norns
