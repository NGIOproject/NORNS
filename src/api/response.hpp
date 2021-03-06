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

#ifndef __API_RESPONSE_HPP__
#define  __API_RESPONSE_HPP__

#include <memory>
#include <vector>
#include <string>

#include "common.hpp"
#include "utils.hpp"

namespace norns {

enum class urd_error;

// forward declarations
namespace io {
    struct task_stats;
    struct global_stats;
};

namespace rpc {
    class Response;
}

namespace api {

enum class response_type {
    iotask_create,
    iotask_status,
    global_status,
    command,
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
    virtual urd_error error_code() const = 0;
    virtual void set_error_code(urd_error ecode) = 0;
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

    template <typename... Args>
    response_impl(Args... args) 
        : std::tuple<FieldTypes...>(args...),
          m_type(RT) { }

//    response_impl(FieldTypes... fields) 
//        : std::tuple<FieldTypes...>(std::forward<FieldTypes>(fields)...),
//          m_type(RT) { }

    response_type type() const override {
        return m_type;
    }

    urd_error error_code() const override {
        return m_error_code;
    }

    void set_error_code(urd_error ecode) override {
        m_error_code = ecode;
    }

    std::string to_string() const override {
        return utils::to_string(m_error_code);
    }

    void pack_extra_info(norns::rpc::Response& /*r*/) const override {
        // default noop implementation for specializations that
        // don't overload it
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
    urd_error m_error_code;

};

} // namespace detail

/*! Aliases for convenience */
using iotask_create_response = detail::response_impl<
    response_type::iotask_create,
    iotask_id
>;

using iotask_status_response = detail::response_impl<
    response_type::iotask_status,
    io::task_stats
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

using global_status_response = detail::response_impl<
    response_type::global_status,
    io::global_stats
>;

using command_response = detail::response_impl<
    response_type::command
>;

using bad_request_response = detail::response_impl<
    response_type::bad_request
>;

} // namespace api
} // namespace norns

#endif /* __API_RESPONSE_HPP__ */
