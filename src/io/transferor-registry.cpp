#include "resources.hpp"
#include "transferor-registry.hpp"

namespace norns {
namespace io {

bool 
transferor_registry::add(const data::resource_type t1, 
                         const data::resource_type t2, 
                         std::shared_ptr<io::transferor>&& trp) {

    using ValueType = std::shared_ptr<io::transferor>;

    if(m_transferors.count(std::make_pair(t1, t2)) == 0) {
        m_transferors.emplace(std::make_pair(t1, t2), 
                              std::forward<ValueType>(trp));
        return true;
    }

    return false;
}

std::shared_ptr<io::transferor> 
transferor_registry::get(const data::resource_type t1, 
                         const data::resource_type t2) const {

    using ValueType = std::shared_ptr<io::transferor>;

    if(m_transferors.count(std::make_pair(t1, t2)) == 0) {
        return ValueType();
    }

    return m_transferors.at(std::make_pair(t1, t2));
}

} // namespace io
} // namespace norns
