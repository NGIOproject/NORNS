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
