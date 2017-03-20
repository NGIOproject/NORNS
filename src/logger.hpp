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
//

#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <spdlog/spdlog.h>

class logger {

    static const int32_t queue_size = 8192; // must be a power of 2

public:
    logger(const std::string& ident, const std::string& type) {

        try {
            spdlog::set_async_mode(queue_size);

            if(type == "stdout") {
                m_internal_logger = spdlog::stdout_logger_mt(ident);
            }
#ifdef SPDLOG_ENABLE_SYSLOG 
            else if(type == "syslog") {
                m_internal_logger = spd::syslog_logger("syslog", "ident", LOG_PID);
            }
#endif
            else {
                // FIXME: add custom exceptions here!
                abort();
            }

            assert(m_internal_logger != nullptr);

            spdlog::drop_all();

            // globally register the logger so that it can be accessed 
            // using spdlog::get(logger_name)
            spdlog::register_logger(m_internal_logger);
        }
        catch(const spdlog::spdlog_ex& ex) {
            // FIXME: add custom exceptions here!
            std::cerr << "spdlog initialization failed!: " << ex.what() << std::endl;
            abort();
        }
    }

    template <typename... Args>
    void info(const char* fmt, const Args&... args) {
        m_internal_logger->info(fmt, args...);
    }

    template <typename... Args>
    void debug(const char* fmt, const Args&... args) {
        m_internal_logger->debug(fmt, args...);
    }

    template <typename... Args>
    void warn(const char* fmt, const Args&... args) {
        m_internal_logger->warn(fmt, args...);
    }

    template <typename... Args>
    void error(const char* fmt, const Args&... args) {
        m_internal_logger->error(fmt, args...);
    }

    template <typename... Args>
    void critical(const char* fmt, const Args&... args) {
        m_internal_logger->critical(fmt, args...);
    }

private:
    std::shared_ptr<spdlog::logger> m_internal_logger;
    std::string                     m_type;
};

#endif /* __LOGGER_HPP__ */

