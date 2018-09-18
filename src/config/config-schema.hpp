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

#ifndef __CONFIG_SCHEMA_HPP__
#define __CONFIG_SCHEMA_HPP__

#include "file-options.hpp"
#include "parsers.hpp"
#include "keywords.hpp"
#include "defaults.hpp"

namespace norns {
namespace config {

using file_options::file_schema;
using file_options::declare_option;
using file_options::declare_list;
using file_options::declare_group;
using file_options::declare_file;
using file_options::converter;
using file_options::sec_type;
using file_options::opt_type;

// define the configuration file structure and declare the supported options
const file_schema valid_options = declare_file({
    // section for global settings
    declare_section(
        keywords::global_settings, 
        sec_type::mandatory,
        declare_group({   

            declare_option<bool>(
                    keywords::use_syslog, 
                    opt_type::mandatory, 
                    converter<bool>(parsers::parse_bool)), 

            declare_option<bfs::path>(
                    keywords::log_file,
                    opt_type::optional,
                    converter<bfs::path>(parsers::parse_path)), 

            declare_option<uint32_t>(
                    keywords::log_file_max_size,
                    opt_type::optional,
                    converter<uint32_t>(parsers::parse_capacity)),

            declare_option<bool>(
                    keywords::dry_run, 
                    opt_type::optional, 
                    defaults::dry_run,
                    converter<bool>(parsers::parse_bool)), 

            declare_option<bfs::path>(
                    keywords::global_socket, 
                    opt_type::mandatory, 
                    converter<bfs::path>(parsers::parse_path)), 

            declare_option<bfs::path>(
                    keywords::control_socket, 
                    opt_type::mandatory, 
                    converter<bfs::path>(parsers::parse_path)), 

            declare_option<uint32_t>(
                    keywords::remote_port, 
                    opt_type::mandatory, 
                    converter<uint32_t>(parsers::parse_number)), 

            declare_option<bfs::path>(
                    keywords::pidfile, 
                    opt_type::mandatory, 
                    converter<bfs::path>(parsers::parse_path)), 

            declare_option<uint32_t>(
                    keywords::workers, 
                    opt_type::mandatory, 
                    converter<uint32_t>(parsers::parse_number)), 
        })
    ),

    // section for namespaces
    declare_section(
        keywords::namespaces, 
        sec_type::optional,
        declare_list({
            declare_option<std::string>(
                    keywords::nsid,
                    opt_type::mandatory),
            declare_option<bool>(
                    keywords::track_contents,
                    opt_type::mandatory,
                    converter<bool>(parsers::parse_bool)), 
            declare_option<bfs::path>(
                    keywords::mountpoint,
                    opt_type::mandatory,
                    converter<bfs::path>(parsers::parse_existing_path)),
            declare_option<std::string>(
                    keywords::type,
                    opt_type::mandatory),
            declare_option<uint64_t>(
                    keywords::capacity,
                    opt_type::mandatory,
                    converter<uint64_t>(parsers::parse_capacity)),
            declare_option<std::string>(
                    keywords::visibility,
                    opt_type::mandatory)
        })
    )
});

} // namespace config
} // namespace norns

#endif /* __CONFIG_SCHEMA_HPP__ */
