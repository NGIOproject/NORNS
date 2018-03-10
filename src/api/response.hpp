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

#ifndef __API_RESPONSE_HPP__
#define  __API_RESPONSE_HPP__

#include <memory>
#include <vector>
#include <string>

#include "norns.h"
#include "utils.hpp"

// forward declarations
namespace io {
    struct task_stats;
    struct task_stats_view;
};

namespace norns { namespace rpc {
    class Response;
}}

namespace api {

enum class response_type {
    iotask_create,
    iotask_status,
    ping,
    job_register, 
    job_update,
    job_unregister,
    process_register,
    process_unregister,
    backend_register, 
    backend_update,
    backend_unregister,
    bad_request
};

/*! Base virtual class for responses */
struct response {

    // aliases for convenience
    using response_ptr = std::unique_ptr<response>;

    virtual ~response() {}
    virtual response_type type() const = 0;
    virtual uint32_t error_code() const = 0;
    virtual void set_error_code(uint32_t ecode) = 0;
    virtual std::string to_string() const = 0;
    virtual void pack_extra_info(norns::rpc::Response& r) const = 0;

    static bool store_to_buffer(response_ptr response, std::vector<uint8_t>& buffer);
    static void cleanup();
};

// concrete implementation details
namespace detail {

/*! Template for concrete implementations of api::response subtypes */ 
template <response_type RT, typename... FieldTypes>
struct response_impl : std::tuple<FieldTypes...>, response {

    response_impl(FieldTypes... fields) 
        : std::tuple<FieldTypes...>(std::forward<FieldTypes>(fields)...),
          m_type(RT) { }

    response_type type() const override {
        return m_type;
    }

    uint32_t error_code() const override {
        return m_error_code;
    }

    void set_error_code(uint32_t ecode) override {
        m_error_code = ecode;
    }

    std::string to_string() const override {
        return utils::strerror(m_error_code);
    }

    void pack_extra_info(norns::rpc::Response& /*r*/) const override {
    }

    template <std::size_t I>
    typename std::tuple_element<I, std::tuple<FieldTypes...>>::type get() const {
        return std::get<I>(*this);
    }

    template <std::size_t I>
    void set(typename std::tuple_element<I, std::tuple<FieldTypes...>>::type v) {
        std::get<I>(*this) = v;
    }

    response_type m_type;
    uint32_t m_error_code;
};

} // namespace detail

/*! Aliases for convenience */
using iotask_create_response = detail::response_impl<
    response_type::iotask_create,
    norns_tid_t
>;

using iotask_status_response = detail::response_impl<
    response_type::iotask_status,
    io::task_stats_view
>;

using ping_response = detail::response_impl<
    response_type::ping
>;

using job_register_response = detail::response_impl<
    response_type::job_register
>;

using job_update_response = detail::response_impl<
    response_type::job_update
>;

using job_unregister_response = detail::response_impl<
    response_type::job_unregister
>;

using process_register_response = detail::response_impl<
    response_type::process_register
>;

using process_unregister_response = detail::response_impl<
    response_type::process_unregister
>;

using backend_register_response = detail::response_impl<
    response_type::backend_register
>;

using backend_update_response = detail::response_impl<
    response_type::backend_update
>;

using backend_unregister_response = detail::response_impl<
    response_type::backend_unregister
>;

using bad_request_response = detail::response_impl<
    response_type::bad_request
>;

} // namespace api

#endif /* __API_RESPONSE_HPP__ */
