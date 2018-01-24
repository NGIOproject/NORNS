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

#ifndef __BACKENDS_HPP__
#define __BACKENDS_HPP__

#include <functional>
#include <unordered_map>
#include <memory>
#include <boost/preprocessor/cat.hpp>

#include <iostream>

namespace storage {

class backend {
public:
    virtual ~backend() {};

    virtual std::string mount() const = 0;
    virtual uint32_t quota() const = 0;
    virtual void read_data() const = 0;
    virtual void write_data() const = 0;

    virtual std::string to_string() const = 0;

}; // class backend

#define NORNS_REGISTER_BACKEND(id, T)                                                 \
    static bool BOOST_PP_CAT(T, __regged) =                                           \
        storage::backend_factory::get_instance().register_backend<T>(id,              \
                [](const std::string& mount, uint32_t quota) {                        \
                    return std::shared_ptr<T>(new T(mount, quota)); \
                });

class backend_factory {

    using creator_function = std::function<std::shared_ptr<backend>(const std::string&, uint32_t)>;


public:
    static backend_factory& get_instance();

    template <typename ...Args>
    static std::shared_ptr<backend> create_from(Args&& ...args) {
        return get_instance().create(std::forward<Args>(args)...);
    }

    template <typename T>
    bool register_backend(const int32_t id, creator_function fn) {

        if(m_registrar.find(id) != m_registrar.end()){
            throw std::invalid_argument("A storage backend with that name already exists!");
        }

        m_registrar.insert({id, fn});
        return true;
    }

private:
    std::shared_ptr<backend> create(int32_t type, const std::string& mount, uint32_t quota) const;

protected:
    backend_factory() {}
    backend_factory(const backend_factory&) {}
    backend_factory& operator=(const backend_factory&); //{}
    ~backend_factory() {}

private:
    std::unordered_map<int32_t, creator_function> m_registrar;

}; // class factory

} // namespace storage

#endif // __BACKENDS_HPP__
