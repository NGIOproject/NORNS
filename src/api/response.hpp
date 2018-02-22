/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 *                                                                       *
 * This file is part of the Data Scheduler, a daemon for tracking and    *
 * managing requests for asynchronous data transfer in a hierarchical    *
 * storage environment.                                                  *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The Data Scheduler is free software: you can redistribute it and/or   *
 * modify it under the terms of the GNU Lesser General Public License    *
 * as published by the Free Software Foundation, either version 3 of     *
 * the License, or (at your option) any later version.                   *
 *                                                                       *
 * The Data Scheduler is distributed in the hope that it will be useful, *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with Data Scheduler.  If not, see                *
 * <http://www.gnu.org/licenses/>.                                       *
 *                                                                       *
 *************************************************************************/

#ifndef __API_RESPONSE_HPP__
#define  __API_RESPONSE_HPP__

#include <memory>
#include <vector>
#include <string>

#include "norns.h"
#include "utils.hpp"

namespace api {

enum class response_type {
    transfer_task,
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
    virtual uint32_t status() const = 0;
    virtual void set_status(uint32_t status) = 0;
    virtual std::string to_string() const = 0;

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

    uint32_t status() const override {
        return m_status;
    }

    void set_status(uint32_t status) override {
        m_status = status;
    }

    std::string to_string() const override {
        return utils::strerror(m_status);
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
    uint32_t m_status;
};

} // namespace detail

/*! Aliases for convenience */
using transfer_task_response = detail::response_impl<
    response_type::transfer_task,
    norns_tid_t
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
