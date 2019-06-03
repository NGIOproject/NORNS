#ifndef __TRANSFEROR_BASE_HPP__
#define __TRANSFEROR_BASE_HPP__

#include <string>
#include <system_error>

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

struct task_info;

struct transferor {

    virtual bool
    validate(const std::shared_ptr<data::resource_info> &src_info,
             const std::shared_ptr<data::resource_info> &dst_info) const = 0;

    virtual std::error_code
    transfer(const auth::credentials &auth,
             const std::shared_ptr<task_info> &task_info,
             const std::shared_ptr<const data::resource> &src,
             const std::shared_ptr<const data::resource> &dst) const = 0;

    virtual std::error_code
    accept_transfer(const auth::credentials &auth,
                    const std::shared_ptr<task_info> &task_info,
                    const std::shared_ptr<const data::resource> &src,
                    const std::shared_ptr<const data::resource> &dst) const = 0;

    virtual std::string 
    to_string() const = 0;
};

} // namespace io
} // namespace norns

#endif /* __TRANSFEROR_BASE_HPP__ */
