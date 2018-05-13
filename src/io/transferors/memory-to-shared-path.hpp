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

#ifndef __IO_MEM_TO_SHARED_PATH_TX__
#define __IO_MEM_TO_SHARED_PATH_TX__

#include <memory>
#include <system_error>
#include "transferor.hpp"

namespace norns {

// forward declarations
namespace auth {
struct credentials;
}

namespace data {
struct resource_info;
struct resource;
}

namespace io {

struct memory_region_to_shared_path_transferor : public transferor {
    bool validate(const std::shared_ptr<data::resource_info>& src_info,
                  const std::shared_ptr<data::resource_info>& dst_info) const override final;
    std::error_code transfer(const auth::credentials& usr_creds,                
                             const std::shared_ptr<const data::resource>& src,  
                             const std::shared_ptr<const data::resource>& dst) const override final;
};

} // namespace io
} // namespace norns

#endif /* __IO_MEM_TO_SHARED_PATH_TX__ */
