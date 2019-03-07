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


#ifndef __LOGGER_HPP__
#define __LOGGER_HPP__

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/bundled/format.h>
#include <boost/filesystem.hpp>
#include <sstream>

#if FMT_VERSION < 50000
namespace fmt {
template <typename T>
inline const void *ptr(const T *p) { return p; }
} // namespace fmt
#endif // FMT_VERSION

namespace bfs = boost::filesystem;

class logger {

    static const int32_t queue_size = 8192; // must be a power of 2

public:
    logger(const std::string& ident, const std::string& type, 
           const bfs::path& log_file="") {

        try {

            if(type == "console") {
                spdlog::set_async_mode(queue_size);
                m_internal_logger = spdlog::stdout_logger_mt(ident);
            }
            else if(type == "console color") {
                spdlog::set_async_mode(queue_size);
                m_internal_logger = spdlog::stdout_color_mt(ident);
            }
            else if(type == "file") {
                spdlog::set_async_mode(queue_size);
                m_internal_logger = spdlog::basic_logger_mt(ident, 
                        log_file.string());
            }
#ifdef SPDLOG_ENABLE_SYSLOG 
            else if(type == "syslog") {
                m_internal_logger = spdlog::syslog_logger("syslog", ident, LOG_PID);
            }
#endif
            else {
                throw std::invalid_argument("Unknown logger type: '" + type + "'");
            }

            assert(m_internal_logger != nullptr);

            // %Y - Year in 4 digits
            // %m - month 1-12
            // %d - day 1-31
            // %T - ISO 8601 time format (HH:MM:SS)
            // %f - microsecond part of the current second
            // %E - epoch (microseconds precision)
            // %n - logger's name
            // %t - thread id
            // %l - log level
            // %v - message
            //m_internal_logger->set_pattern("[%Y-%m-%d %T.%f] [%E] [%n] [%t] [%l] %v");
            m_internal_logger->set_pattern("[%Y-%m-%d %T.%f] [%n] [%t] [%l] %v");

#ifdef __LOGGER_ENABLE_DEBUG__
            enable_debug();
#endif

            spdlog::drop_all();

            // globally register the logger so that it can be accessed 
            // using spdlog::get(logger_name)
            //spdlog::register_logger(m_internal_logger);
        }
        catch(const spdlog::spdlog_ex& ex) {
            throw std::runtime_error("logger initialization failed: " + std::string(ex.what()));
        }
    }

    logger(const logger& /*rhs*/) = delete;
    logger& operator=(const logger& /*rhs*/) = delete;
    logger(logger&& /*other*/) = default;
    logger& operator=(logger&& /*other*/) = default;

    ~logger() {
        spdlog::drop_all();
    }

    // the following static functions can be used to interact 
    // with a globally registered logger instance

    template <typename... Args>
    static inline void create_global_logger(Args&&... args) {
        global_logger() = std::make_shared<logger>(args...);
    }

    static inline void register_global_logger(logger&& lg) {
        global_logger() = std::make_shared<logger>(std::move(lg));
    }

    static inline std::shared_ptr<logger>& get_global_logger() {
        return global_logger();
    }

// some macros to make it more convenient to use the global logger
#define LOGGER_INFO(...)                                    \
do {                                                        \
    if(logger::get_global_logger()) {                       \
        logger::get_global_logger()->info(__VA_ARGS__);     \
    }                                                       \
} while(0);


#ifdef __LOGGER_ENABLE_DEBUG__

#define LOGGER_DEBUG(...) \
do {                                                        \
    if(logger::get_global_logger()) {                       \
        logger::get_global_logger()->debug(__VA_ARGS__);    \
    }                                                       \
} while(0);

#define LOGGER_FLUSH() \
do {                                                        \
    if(logger::get_global_logger()) {                       \
        logger::get_global_logger()->flush();               \
    }                                                       \
} while(0);

#else

#define LOGGER_DEBUG(...) do {} while(0);
#define LOGGER_FLUSH() do {} while(0);

#endif // __LOGGER_ENABLE_DEBUG__

#define LOGGER_WARN(...) \
do {                                                        \
    if(logger::get_global_logger()) {                       \
        logger::get_global_logger()->warn(__VA_ARGS__);     \
    }                                                       \
} while(0);

#define LOGGER_ERROR(...) \
do {                                                        \
    if(logger::get_global_logger()) {                       \
        logger::get_global_logger()->error(__VA_ARGS__);    \
    }                                                       \
} while(0);

#define LOGGER_ERRNO(...) \
do {                                                        \
    if(logger::get_global_logger()) {                       \
    logger::get_global_logger()->error_errno(__VA_ARGS__);  \
    }                                                       \
} while(0);

#define LOGGER_CRITICAL(...) \
do {                                                        \
    if(logger::get_global_logger()) {                       \
        logger::get_global_logger()->critical(__VA_ARGS__); \
    }                                                       \
} while(0);

    // the following member functions can be used to interact 
    // with a specific logger instance
    inline void enable_debug() const {
        m_internal_logger->set_level(spdlog::level::debug);
    }

    inline void flush() {
        m_internal_logger->flush();
    }

    template <typename... Args>
    inline void info(const char* fmt, const Args&... args) {
        m_internal_logger->info(fmt, args...);
    }

    template <typename... Args>
    inline void debug(const char* fmt, const Args&... args) {
        m_internal_logger->debug(fmt, args...);
    }

    template <typename... Args>
    inline void warn(const char* fmt, const Args&... args) {
        m_internal_logger->warn(fmt, args...);
    }

    template <typename... Args>
    inline void error(const char* fmt, const Args&... args) {
        m_internal_logger->error(fmt, args...);
    }

    static inline std::string
    errno_message(int errno_value) {
        // 1024 should be more than enough for most locales
        constexpr const std::size_t MAX_ERROR_MSG = 1024;
        std::array<char, MAX_ERROR_MSG> errstr;
        char* msg = strerror_r(errno_value, errstr.data(), MAX_ERROR_MSG);
        return std::string{msg};
    }

    template <typename... Args>
    inline void error_errno(const char* fmt, const Args&... args) {

        const int saved_errno = errno;

        constexpr const std::size_t MAX_ERROR_MSG = 128;
        std::array<char, MAX_ERROR_MSG> errstr;

        std::string str_fmt(fmt);
        str_fmt += ": {}";

        char* msg = strerror_r(saved_errno, errstr.data(), MAX_ERROR_MSG);

        m_internal_logger->error(str_fmt.c_str(), args..., msg);
    }

    template <typename... Args>
    inline void critical(const char* fmt, const Args&... args) {
        m_internal_logger->critical(fmt, args...);
    }

    template <typename T>
    inline void info(const T& msg) {
        m_internal_logger->info(msg);
    }

    template <typename T>
    inline void debug(const T& msg) {
        m_internal_logger->debug(msg);
    }

    template <typename T>
    inline void warn(const T& msg) {
        m_internal_logger->warn(msg);
    }

    template <typename T>
    inline void error(const T& msg) {
        m_internal_logger->error(msg);
    }

    template <typename T>
    inline void critical(const T& msg) {
        m_internal_logger->critical(msg);
    }

    template <typename... Args>
    static inline std::string build_message(Args&&... args) {

        // see:
        // 1. https://stackoverflow.com/questions/27375089/what-is-the-easiest-way-to-print-a-variadic-parameter-pack-using-stdostream
        // 2. https://stackoverflow.com/questions/25680461/variadic-template-pack-expansion/25683817#25683817

        std::stringstream ss;

        using expander = int[];
        (void) expander{0, (void(ss << std::forward<Args>(args)),0)...};

        return ss.str();
    }

private:

    static std::shared_ptr<logger>& global_logger() {
        static std::shared_ptr<logger> s_global_logger;
        return s_global_logger;
    }

private:
    std::shared_ptr<spdlog::logger> m_internal_logger;
    std::string                     m_type;
};

// override the logging macros of the hermes library
// and replace it with ours, which use the same fmt definitions
// to format message strings
#ifdef HERMES_INFO 
#undef HERMES_INFO 
#endif
#define HERMES_INFO(...)     LOGGER_INFO(__VA_ARGS__)

#ifdef HERMES_WARNING 
#undef HERMES_WARNING
#endif
#define HERMES_WARNING(...)  LOGGER_WARN(__VA_ARGS__)

#ifdef HERMES_DEBUG
#undef HERMES_DEBUG
#endif
#define HERMES_DEBUG(...)    LOGGER_DEBUG(__VA_ARGS__)

#ifdef HERMES_DEBUG2
#undef HERMES_DEBUG2
#endif
#define HERMES_DEBUG2(...)   LOGGER_DEBUG(__VA_ARGS__)

#ifdef HERMES_DEBUG3
#undef HERMES_DEBUG3
#endif
#define HERMES_DEBUG3(...)   LOGGER_DEBUG(__VA_ARGS__)

#ifdef HERMES_DEBUG4
#undef HERMES_DEBUG4
#endif
#define HERMES_DEBUG4(...)   // LOGGER_DEBUG(__VA_ARGS__)

#ifdef HERMES_ERROR
#undef HERMES_ERROR
#endif
#define HERMES_ERROR(...)    LOGGER_ERROR(__VA_ARGS__)

#ifdef HERMES_FATAL
#undef HERMES_FATAL
#endif
#define HERMES_FATAL(...)    LOGGER_CRITICAL(__VA_ARGS__)

#include <hermes/logging.hpp>

#endif /* __LOGGER_HPP__ */
