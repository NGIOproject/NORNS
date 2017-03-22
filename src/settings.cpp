//
// Copyright (C) 2017 Barcelona Supercomputing Center
//                    Centro Nacional de Supercomputacion
//
// This file is part of the Data Scheduler, a daemon for tracking and managing
// requests for asynchronous data transfer in a hierarchical storage environment.
//
// See AUTHORS file in the top level directory for information
// regarding developers and contributors.
//
// The Data Scheduler is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Data Scheduler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Data Scheduler.  If not, see <http://www.gnu.org/licenses/>.
//

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "settings.hpp"

namespace bpt = boost::property_tree;

static uint64_t parse_size(const std::string& str);

void config_settings::load(const std::string& filename) {

    bpt::ptree pt;

    bpt::read_json(filename, pt);

    m_progname = defaults::progname;

    m_running_dir = pt.get<std::string>("settings.running_dir", "");

    if(m_running_dir == ""){
        m_running_dir = defaults::running_dir;
    }

    m_ipc_sockfile = defaults::ipc_sockfile;
    
    m_daemon_pidfile = pt.get<std::string>("settings.pidfile", "");

    if(m_daemon_pidfile == ""){
        m_daemon_pidfile = defaults::daemon_pidfile;
    }

    m_workers_in_pool = pt.get<int32_t>("settings.workers", 0);

    if(m_workers_in_pool == 0){
        m_workers_in_pool = defaults::workers_in_pool;
    }

    m_storage_path = pt.get<std::string>("settings.storage.path", "");

    if(m_storage_path == "") {
        std::cerr << "Missing storage.path setting in configuration file";
        exit(EXIT_FAILURE);
    }

    auto capacity_str = pt.get<std::string>("settings.storage.capacity", "");

    if(capacity_str == "") {
        std::cerr << "Missing storage.capacity setting in configuration file";
        exit(EXIT_FAILURE);
    }

    try {
        m_storage_capacity = parse_size(capacity_str);
    } catch(std::invalid_argument ex) {
        std::cerr << "Unable to parse storage.capacity parameter '" << capacity_str << "': " << ex.what() << "\n";
        exit(EXIT_FAILURE);
    }

    /* parse storage backends */
    BOOST_FOREACH(bpt::ptree::value_type& bend, pt.get_child("backends")){
        // bend.first == ""
        const auto& name = bend.second.get<std::string>("name", "");

        if(name == ""){
            std::cerr << "Missing 'name' in backend description";
            exit(EXIT_FAILURE);
        }

        const auto& path = bend.second.get<std::string>("path", "");

        if(path == ""){
            std::cerr << "Missing 'path' in backend description";
            exit(EXIT_FAILURE);
        }

        const auto& capacity_str = bend.second.get<std::string>("capacity", "");

        if(capacity_str == ""){
            std::cerr << "Missing 'capacity' in backend description";
            exit(EXIT_FAILURE);
        }

        uint64_t capacity;

        try {
            capacity = parse_size(capacity_str);
        } catch(std::invalid_argument ex) {
            std::cerr << "Error parsing backend 'capacity' parameter: '" << capacity_str << "': " << ex.what() << "\n";
            exit(EXIT_FAILURE);
        }

        m_backends.emplace_back(name, path, capacity);
    }
}

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
