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

#ifndef __IO_TRANSFEROR_REGISTRY_HPP__
#define __IO_TRANSFEROR_REGISTRY_HPP__

#include <system_error>
#include <utility>
#include <boost/functional/hash.hpp>
#include "auth/process-credentials.hpp"
#include "common.hpp"

namespace norns {

// forward declarations
namespace data { 
enum class resource_type;
}

namespace io {

struct transferor_registry {

    struct transferor_hash {
        template <typename T, typename U>
        std::size_t operator()(const std::pair<T, U> &x) const {
            std::size_t seed = 0;
            boost::hash_combine(seed, x.first);
            boost::hash_combine(seed, x.second);
            return seed;
        }
    };

    using SrcType = const data::resource_type;
    using DstType = const data::resource_type;
    using CredType = const auth::credentials&;
    using ArgType = const std::shared_ptr<const data::resource>&;
    using ReturnType = std::error_code;
    using KeyType = std::pair<SrcType, DstType>;
    using KeyHash = transferor_hash;
    using DispatcherType = dispatch_table<KeyType, KeyHash, ReturnType, 
                                          CredType, ArgType, ArgType>;
    using CallableType = DispatcherType::CallableType;

    void add(SrcType t1, DstType t2, CallableType&& func);
    CallableType get(SrcType t1, DstType t2) const;
    ReturnType invoke(SrcType t1, DstType t2, CredType creds, 
                      ArgType arg1, ArgType arg2) const;

    DispatcherType m_dispatcher;
};

} // namespace io
} // namespace norns

#endif /* __IO_TRANSFEROR_REGISTRY_HPP__ */

