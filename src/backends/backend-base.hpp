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

#ifndef __BACKEND_BASE_HPP__
#define __BACKEND_BASE_HPP__

#include <functional>
#include <unordered_map>
#include <memory>
#include <boost/preprocessor/cat.hpp>

//#include "resources/resource-info.hpp"
#include "resources/resource.hpp"

namespace norns {
namespace storage {

class backend {

protected:
    using resource_info_ptr = std::shared_ptr<data::resource_info>;

public:
    virtual ~backend() {};

    virtual std::string mount() const = 0;
    virtual uint32_t quota() const = 0;
    virtual void read_data() const = 0;
    virtual void write_data() const = 0;

    virtual bool accepts(resource_info_ptr res) const = 0;
    virtual bool contains(resource_info_ptr res) const = 0;

    virtual std::string to_string() const = 0;

}; // class backend

#define NORNS_REGISTER_BACKEND(id, T)                                                 \
    static bool BOOST_PP_CAT(T, __regged) =                                           \
        storage::backend_factory::get().register_backend<T>(id,              \
                [](const std::string& mount, uint32_t quota) {                        \
                    return std::shared_ptr<T>(new T(mount, quota)); \
                });

class backend_factory {

    using creator_function = std::function<std::shared_ptr<backend>(const std::string&, uint32_t)>;


public:
    static backend_factory& get();

    template <typename ...Args>
    static std::shared_ptr<backend> create_from(Args&& ...args) {

        try {
            return get().create(std::forward<Args>(args)...);
        }
        catch(std::invalid_argument) {
            return std::shared_ptr<backend>();
        }
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
} // namespace norns

#endif // __BACKEND_BASE_HPP__
