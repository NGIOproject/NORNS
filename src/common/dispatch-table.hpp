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

/**
 * This file implements a dispatch table that allows clients to register 
 * callables (i.e. lambdas, functions, functors, etc) and associate them
 * to a user-provided key.
 *
 * Example:
 *
 *  dispatch_table<
 *      api::request_type, 
 *      api::request_type_hash,
 *      response_ptr,
 *      request_ptr
 *          > dispatcher;
 *
 *  dispatcher.add(
 *      api::request_type::job_register,
 *      std::bind(&server::job_register, &ss, std::placeholders::_1));
 *
 *  for(int i=0; i<10; ++i) {
 *      auto req = generate_request();
 *
 *      auto resp = dispatcher.run(req->type(), std::move(req));
 *
 *      ...
 *  }
 */

#ifndef __DISPATCH_TABLE_HPP__
#define __DISPATCH_TABLE_HPP__

#include <functional>
#include <unordered_map>

namespace norns {

template <typename Key, typename Hash, typename DispatchReturn, typename... DispatchArgs>
struct dispatch_table {

    using CallableType = std::function<DispatchReturn(DispatchArgs...)>;

    template <typename Callable>
    void add(Key k, Callable&& func) {
        m_callbacks.emplace(k, std::forward<Callable>(func));
    }

    // DispatchReturn run(Key k, const DispatchArgs&... args) {

    //     if(!m_callbacks.count(k)) {
    //         throw std::invalid_argument("Found no callback for provided key");
    //     }

    //     return m_callbacks.at(k)(std::forward<DispatchArgs>(args)...);
    // }

    CallableType get(Key k) const {

        const auto& it = m_callbacks.find(k);

        if(it == m_callbacks.end()) {
            return CallableType();
        }

        return it->second;
    }

    DispatchReturn invoke(Key k, DispatchArgs&&... args) const {

        if(!m_callbacks.count(k)) {
            throw std::invalid_argument("Found no callback for provided key");
        }

        return m_callbacks.at(k)(std::forward<DispatchArgs>(args)...);
    }



    std::unordered_map<Key, CallableType, Hash> m_callbacks;
};

} // namespace norns

#endif /* __DISPATCH_TABLE_HPP__ */
