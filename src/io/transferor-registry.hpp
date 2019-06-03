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

struct transferor;

struct transferor_registry {

    bool add(const data::resource_type t1, 
             const data::resource_type t2, 
             std::shared_ptr<io::transferor>&& tr);
    std::shared_ptr<io::transferor> get(const data::resource_type t1, 
                                        const data::resource_type t2) const;

    using key_type = 
        std::pair<const data::resource_type, const data::resource_type>;

    std::unordered_map<key_type,
                       std::shared_ptr<io::transferor>, 
                       boost::hash<key_type>> m_transferors;
};

} // namespace io
} // namespace norns

#endif /* __IO_TRANSFEROR_REGISTRY_HPP__ */

