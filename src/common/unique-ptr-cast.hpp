#include <memory>

#ifndef __UNIQUE_PTR_CAST_HPP__
#define __UNIQUE_PTR_CAST_HPP__

namespace norns {
namespace utils {

template<typename Derived, typename Base, typename Del>
std::unique_ptr<Derived, Del> 
static_unique_ptr_cast( std::unique_ptr<Base, Del>&& p )
{
    auto d = static_cast<Derived *>(p.release());
    return std::unique_ptr<Derived, Del>(d, std::move(p.get_deleter()));
}

template<typename Derived, typename Base, typename Del>
std::unique_ptr<Derived, Del> 
static_unique_ptr_cast(const std::unique_ptr<Base, Del>&& p )
{
    return static_unique_ptr_cast<Derived, Base, Del>(
            std::move(const_cast<std::unique_ptr<Base, Del>&>(p)));
}

template<typename Derived, typename Base, typename Del>
std::unique_ptr<Derived, Del> 
dynamic_unique_ptr_cast( std::unique_ptr<Base, Del>&& p )
{
    if(Derived *result = dynamic_cast<Derived *>(p.get())) {
        p.release();
        return std::unique_ptr<Derived, Del>(result, std::move(p.get_deleter()));
    }
    return std::unique_ptr<Derived, Del>(nullptr, p.get_deleter());
}

} // namespace utils
} // namespace norns

#endif /* __UNIQUE_PTR_CAST_HPP__ */
