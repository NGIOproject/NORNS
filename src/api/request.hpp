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

#ifndef __API_REQUESTS_HPP__
#define __API_REQUESTS_HPP__

/**
 * This file implements the concepts of API requests, as received from 
 * the client. For convenience, api::request instances behave as C++11
 * std::tuples since, in fact, they inherit from it. 
 *
 * The main difference is that we add a constant field that describes
 * the request type, so that users may know what they are dealing with,
 * and provide some typedefs for convenience. For instance, if a request 
 * of type Foo has been defined as:
 *
 *      using Foo_request = detail::request_impl<int, std::string>;
 *
 * this means that an instance for it can be created with either of the 
 * following statements:
 *
 *      Foo_request req(42, "hello world!");
 *      auto req = std::make_unique<Foo_request>(42, "hello world!");
 *
 * This means that
 * the std::get<0>(req) will return field number 0, std::get<1>(req)
 * will return field number 1, and so on, as is usual with std::tuples.
 *
 */

#include <memory>
#include <vector>
#include <string>
#include <boost/optional.hpp>

#include "common.hpp"
#include "auth/process-credentials.hpp"

namespace norns {

// forward declarations
namespace storage {
    class backend;
}

namespace data {
    struct resource_info;
};

namespace api {

/*! Valid types for an API request */
enum class request_type { 
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

/*! Hashing class for request_type */
struct request_type_hash {
    template <typename T>
    std::size_t operator()(T t) const {
        return static_cast<std::size_t>(t);
    }
};

/*! Base virtual class for requests */
struct request {

    using request_ptr = std::unique_ptr<request>;

    // these typedefs are necessary for dispatch_table
    using key_type = api::request_type;
    using key_hash_type = api::request_type_hash;

    virtual ~request() {}
    virtual request_type type() const = 0;
    virtual boost::optional<auth::credentials> credentials() const = 0;
    virtual void set_credentials(boost::optional<auth::credentials>&& creds) = 0;
    virtual void set_credentials(const boost::optional<auth::credentials>& creds) = 0;
    virtual std::string to_string() const = 0;

    static request_ptr create_from_buffer(const std::vector<uint8_t>& data, int size);
    static void cleanup();
};


// concrete implementation details
namespace detail {

/*
template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
  to_string(std::stringstream& ss, const std::tuple<Tp...>& t)
  { }

template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
  to_string(std::stringstream& ss, const std::tuple<Tp...>& t)
  {
    ss << std::get<I>(t) << std::endl;
    to_string<I + 1, Tp...>(ss, t);
  }
  */


/*! Template for concrete implementations of api::request subtypes */ 
template <request_type RT, typename... FieldTypes>
struct request_impl : std::tuple<FieldTypes...>, request {

    request_impl(FieldTypes... fields) 
        : std::tuple<FieldTypes...>(std::forward<FieldTypes>(fields)...),
          m_type(RT) { }

    request_type type() const override final {
        return m_type;
    }

    boost::optional<auth::credentials> credentials() const override final {
        return m_credentials;
    }

    void set_credentials(boost::optional<auth::credentials>&& creds) override final {
        m_credentials = creds;
    }

    void set_credentials(const boost::optional<auth::credentials>& creds) override final {
        m_credentials = creds;
    }

    // this is the implementation for the generic to_string() 
    // function for any RT that is not known. For known RTs, we 
    // provide concrete specializations in the cpp file
    std::string to_string() const override {
        return "UNKNOWN_REQUEST";
    }

    template <std::size_t I>
    typename std::tuple_element<I, std::tuple<FieldTypes...>>::type get() const {
        return std::get<I>(*this);
    }

    request_type m_type;
    boost::optional<auth::credentials> m_credentials;
};

} // namespace detail

/*! Aliases for convenience */
using bad_request = detail::request_impl<
    request_type::bad_request
>;

using iotask_create_request = detail::request_impl<
    request_type::iotask_create,
    iotask_type,
    std::shared_ptr<data::resource_info>,
    std::shared_ptr<data::resource_info>
>;

using iotask_status_request = detail::request_impl<
    request_type::iotask_status,
    iotask_id
>;

using ping_request = detail::request_impl<
    request_type::ping
>;

using job_register_request = detail::request_impl<
    request_type::job_register, 
    uint32_t, 
    std::vector<std::string>, 
    std::vector<std::shared_ptr<storage::backend>>
>;

using job_update_request = detail::request_impl<
    request_type::job_update, 
    uint32_t,
    std::vector<std::string>, 
    std::vector<std::shared_ptr<storage::backend>>
>;

using job_unregister_request = detail::request_impl<
    request_type::job_unregister, 
    uint32_t
>;

using process_register_request = detail::request_impl<
    request_type::process_register, 
    uint32_t,
    uid_t,
    gid_t,
    pid_t
>;

using process_unregister_request = detail::request_impl<
    request_type::process_unregister,
    uint32_t,
    uid_t,
    gid_t,
    pid_t
>;

using backend_register_request = detail::request_impl<
    request_type::backend_register, 
    std::string, // nsid
    backend_type, // type
    std::string, // mount
    int32_t // quota
>;

using backend_update_request = detail::request_impl<
    request_type::backend_update, 
    std::string, // nsid
    backend_type, // type
    std::string, // mount
    int32_t // quota
>;

using backend_unregister_request = detail::request_impl<
    request_type::backend_unregister, 
    std::string
>;

} // namespace api
} // namespace norns

#endif /* __API_REQUESTS_H__ */
