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
#include <boost/property_tree/ptree.hpp>

namespace bpt = boost::property_tree;

namespace storage {

class backend {
public:
    virtual ~backend() {};
    virtual const std::string& get_name() const = 0;
    virtual const std::string& get_type() const = 0;
    virtual const std::string& get_description() const = 0;
    virtual uint64_t get_capacity() const = 0;
    virtual void read_data() const = 0;
    virtual void write_data() const = 0;
}; // class backend

#define REGISTER_BACKEND(name, T)                                                       \
    static bool BOOST_PP_CAT(T, __regged) =                                             \
        storage::backend_factory::get_instance().register_backend<T>(name,              \
                [](const bpt::ptree& options) {                                         \
                    return std::shared_ptr<T>(new T(options)); \
                });

class backend_factory {

    using creatorfn_t = std::function<std::shared_ptr<backend>(const bpt::ptree&)>;


public:
    static backend_factory& get_instance();
    std::shared_ptr<backend> create(const std::string& name, const bpt::ptree& options) const;

    template <typename T>
    bool register_backend(const std::string& name, creatorfn_t fn) {

        if(m_registrar.find(name) != m_registrar.end()){
            throw std::invalid_argument("A storage backend with that name already exists!");
        }

        m_registrar.insert({name, fn});
        return true;
    }

protected:
    backend_factory() {}
    backend_factory(const backend_factory&) {}
    backend_factory& operator=(const backend_factory&); //{}
    ~backend_factory() {}

private:
    std::unordered_map<std::string, creatorfn_t> m_registrar;

}; // class factory

} // namespace storage

#endif // __BACKENDS_HPP__
