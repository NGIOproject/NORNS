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

#ifndef __SETTINGS_HPP__
#define __SETTINGS_HPP__

#include <list>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace bpt = boost::property_tree;

struct config_settings {

    struct backend {

        backend(const std::string& name, const std::string& path, const uint64_t capacity)
            : m_name(name),
              m_path(path),
              m_capacity(capacity){ }

        const std::string m_name;
        const std::string m_path;
        const uint64_t    m_capacity;
    };

    void load(const std::string& filename);

    std::string        m_storage_path;     /* path to internal storage */
    uint64_t           m_storage_capacity; /* internal storage's max capacity */
    std::list<backend> m_backends;         /* list of backend descriptions */
};


#endif /* __SETTINGS_HPP__ */
