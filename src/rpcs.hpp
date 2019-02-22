#ifndef __NORNS_IO_TRANSFERORS_RPCS_HPP__
#define __NORNS_IO_TRANSFERORS_RPCS_HPP__

// C includes
#include <mercury.h>
#include <mercury_proc_string.h>
#include <mercury_macros.h>

// C++ includes
#include <string>

// hermes includes
#include <hermes.hpp>

#ifndef HG_GEN_PROC_NAME
#define HG_GEN_PROC_NAME(struct_type_name) \
    hermes::detail::hg_proc_ ## struct_type_name
#endif

// forward declarations
namespace hermes { namespace detail {

template <typename ExecutionContext>
hg_return_t post_to_mercury(ExecutionContext* ctx);

}} // namespace hermes::detail

//==============================================================================
// definitions for norns::rpc::remote_transfer
namespace hermes { namespace detail {

// Generate Mercury types and serialization functions (field names match
// those defined by remote_transfer::input and remote_transfer::output). These
// definitions are internal and should not be used directly. Classes
// remote_transfer::input and remote_transfer::output are provided for public use.
MERCURY_GEN_PROC(remote_transfer_in_t,
        ((hg_const_string_t) (address))
        ((hg_const_string_t) (in_nsid))
        ((hg_const_string_t) (out_nsid))
        ((uint32_t)          (backend_type))
        ((uint32_t)          (resource_type))
        ((hg_bool_t)         (is_collection))
        ((hg_const_string_t) (resource_name))
        ((hg_bulk_t)         (buffers)))

MERCURY_GEN_PROC(remote_transfer_out_t,
        ((uint32_t) (status))
        ((uint32_t) (task_error))
        ((uint32_t) (sys_errnum))
        ((uint32_t) (elapsed_time)))

MERCURY_GEN_PROC(resource_stat_in_t,
        ((hg_const_string_t) (address))
        ((hg_const_string_t) (nsid))
        ((uint32_t)          (resource_type))
        ((hg_const_string_t) (resource_name)))

MERCURY_GEN_PROC(resource_stat_out_t,
        ((uint32_t) (return_value))
        ((uint64_t) (packed_size)))

}} // namespace hermes::detail


namespace norns {
namespace rpc {

struct remote_transfer {

    // forward declarations of public input/output types for this RPC
    class input;
    class output;

    // traits used so that the engine knows what to do with the RPC
    using self_type = remote_transfer;
    using handle_type = hermes::rpc_handle<self_type>;
    using input_type = input;
    using output_type = output;
    using mercury_input_type = hermes::detail::remote_transfer_in_t;
    using mercury_output_type = hermes::detail::remote_transfer_out_t;

    // RPC public identifier
    constexpr static const uint16_t public_id = 43;

    // RPC internal Mercury identifier
    constexpr static const uint16_t mercury_id = public_id;

    // RPC name
    constexpr static const auto name = "remote_transfer";

    // requires response?
    constexpr static const auto requires_response = true;

    // Mercury callback to serialize input arguments
    constexpr static const auto mercury_in_proc_cb = 
        HG_GEN_PROC_NAME(remote_transfer_in_t);

    // Mercury callback to serialize output arguments
    constexpr static const auto mercury_out_proc_cb = 
        HG_GEN_PROC_NAME(remote_transfer_out_t);

    class input {

        template <typename ExecutionContext>
        friend hg_return_t hermes::detail::post_to_mercury(ExecutionContext*);

    public:
        input(const std::string& address,
              const std::string& in_nsid,
              const std::string& out_nsid,
              uint32_t backend_type,
              uint32_t resource_type,
              uint32_t is_collection,
              const std::string& resource_name,
              const hermes::exposed_memory& buffers) :
            m_address(address),
            m_in_nsid(in_nsid),
            m_out_nsid(out_nsid),
            m_backend_type(backend_type),
            m_resource_type(resource_type),
            m_is_collection(is_collection),
            m_resource_name(resource_name),
            m_buffers(buffers) {

#ifdef HERMES_DEBUG_BUILD
            this->print("this", __PRETTY_FUNCTION__);
#endif

        }

#ifdef HERMES_DEBUG_BUILD
        input(input&& rhs) :
            m_address(std::move(rhs.m_address)),
            m_in_nsid(std::move(rhs.m_in_nsid)),
            m_out_nsid(std::move(rhs.m_out_nsid)),
            m_backend_type(std::move(rhs.m_backend_type)),
            m_resource_type(std::move(rhs.m_resource_type)),
            m_is_collection(std::move(rhs.m_is_collection)),
            m_resource_name(std::move(rhs.m_resource_name)),
            m_buffers(std::move(rhs.m_buffers)) {

            rhs.m_backend_type = 0;
            rhs.m_resource_type = 0;
            rhs.m_is_collection = false;

            this->print("this", __PRETTY_FUNCTION__);
            rhs.print("rhs", __PRETTY_FUNCTION__);
        }

        input(const input& other) :
            m_address(other.m_address),
            m_in_nsid(other.m_in_nsid),
            m_out_nsid(other.m_out_nsid),
            m_backend_type(other.m_backend_type),
            m_resource_type(other.m_resource_type),
            m_is_collection(other.m_is_collection),
            m_resource_name(other.m_resource_name),
            m_buffers(other.m_buffers) {

            this->print("this", __PRETTY_FUNCTION__);
            other.print("other", __PRETTY_FUNCTION__);
        }

        input& 
        operator=(input&& rhs) {

            if(this != &rhs) {
                m_address = std::move(rhs.m_address);
                m_in_nsid = std::move(rhs.m_in_nsid);
                m_out_nsid = std::move(rhs.m_out_nsid);
                m_backend_type = std::move(rhs.m_backend_type);
                m_resource_type = std::move(rhs.m_resource_type);
                m_is_collection = std::move(rhs.m_is_collection);
                m_resource_name = std::move(rhs.m_resource_name);
                m_buffers = std::move(rhs.m_buffers);

                rhs.m_backend_type = 0;
                rhs.m_resource_type = 0;
                rhs.m_is_collection = false;
            }

            this->print("this", __PRETTY_FUNCTION__);
            rhs.print("rhs", __PRETTY_FUNCTION__);

            return *this;
        }

        input& 
        operator=(const input& other) {
            
            if(this != &other) {
                m_address = other.m_address;
                m_in_nsid = other.m_in_nsid;
                m_out_nsid = other.m_out_nsid;
                m_backend_type = other.m_backend_type;
                m_resource_type = other.m_resource_type;
                m_is_collection = other.m_is_collection;
                m_resource_name = other.m_resource_name;
                m_buffers = other.m_buffers;
            }

            this->print("this", __PRETTY_FUNCTION__);
            other.print("other", __PRETTY_FUNCTION__);

            return *this;
        }
#else // HERMES_DEBUG_BUILD
        input(input&& rhs) = default;
        input(const input& other) = default;
        input& operator=(input&& rhs) = default;
        input& operator=(const input& other) = default;
#endif // ! HERMES_DEBUG_BUILD

        std::string
        address() const {
            return m_address;
        }

        std::string
        in_nsid() const {
            return m_in_nsid;
        }

        std::string
        out_nsid() const {
            return m_out_nsid;
        }

        uint32_t
        backend_type() const {
            return m_backend_type;
        }

        uint32_t
        resource_type() const {
            return m_resource_type;
        }

        bool
        is_collection() const {
            return m_is_collection;
        }

        std::string
        resource_name() const {
            return m_resource_name;
        }

        hermes::exposed_memory
        buffers() const {
            return m_buffers;
        }

#ifdef HERMES_DEBUG_BUILD
        void
        print(const std::string& id,
              const std::string& caller = "") const {

            (void) id;
            auto c = caller.empty() ? "unknown_caller" : caller;

            HERMES_DEBUG2("{}, {} ({}) = {{", caller, id, fmt::ptr(this));
            HERMES_DEBUG2("  m_address: \"{}\" ({} -> {}),", 
                         m_address, fmt::ptr(&m_address), 
                         fmt::ptr(m_address.c_str()));
            HERMES_DEBUG2("  m_in_nsid: \"{}\" ({} -> {}),", 
                         m_in_nsid, fmt::ptr(&m_in_nsid),
                         fmt::ptr(m_in_nsid.c_str()));
            HERMES_DEBUG2("  m_out_nsid: \"{}\" ({} -> {}),", 
                         m_out_nsid, fmt::ptr(&m_out_nsid),
                         fmt::ptr(m_out_nsid.c_str()));
            HERMES_DEBUG2("  m_backend_type: {},", 
                         m_backend_type); 
            HERMES_DEBUG2("  m_resource_type: {},", 
                         m_resource_type); 
            HERMES_DEBUG2("  m_is_collection: {},",
                          m_is_collection);
            HERMES_DEBUG2("  m_resource_name: \"{}\" ({} -> {}),", 
                         m_resource_name, fmt::ptr(&m_resource_name),
                         fmt::ptr(m_resource_name.c_str()));
            HERMES_DEBUG2("  m_buffers: {...},"); 
            HERMES_DEBUG2("}}");
        }
#endif // ! HERMES_DEBUG_BUILD

//TODO: make private
        explicit
        input(const hermes::detail::remote_transfer_in_t& other) :
            m_address(other.address),
            m_in_nsid(other.in_nsid),
            m_out_nsid(other.out_nsid),
            m_backend_type(other.backend_type),
            m_resource_type(other.resource_type),
            m_is_collection(other.is_collection),
            m_resource_name(other.resource_name),
            m_buffers(other.buffers) { 

            HERMES_DEBUG("input::input(const hermes::detail::remote_transfer_in_t&){{");
            HERMES_DEBUG("  m_address: {} ({}),", 
                         m_address, fmt::ptr(&m_address));
            HERMES_DEBUG("  m_in_nsid: {} ({}),", 
                         m_in_nsid, fmt::ptr(&m_in_nsid));
            HERMES_DEBUG("  m_out_nsid: {} ({}),", 
                         m_out_nsid, fmt::ptr(&m_out_nsid));
            HERMES_DEBUG("  m_backend_type: {},", 
                         m_backend_type); 
            HERMES_DEBUG("  m_resource_type: {},", 
                         m_resource_type); 
            HERMES_DEBUG("  m_is_collection: {},", 
                         m_is_collection); 
            HERMES_DEBUG("  m_buffers: {...},"); 
            HERMES_DEBUG("}}");
        }
        
        explicit
        operator hermes::detail::remote_transfer_in_t() {
            return {m_address.c_str(),
                    m_in_nsid.c_str(), 
                    m_out_nsid.c_str(), 
                    m_backend_type,
                    m_resource_type, 
                    m_is_collection,
                    m_resource_name.c_str(), 
                    hg_bulk_t(m_buffers)};
        }


    private:
        std::string m_address;
        std::string m_in_nsid;
        std::string m_out_nsid;
        uint32_t m_backend_type;
        uint32_t m_resource_type;
        bool m_is_collection;
        std::string m_resource_name;
        hermes::exposed_memory m_buffers;
    };

    class output {

        template <typename ExecutionContext>
        friend hg_return_t hermes::detail::post_to_mercury(ExecutionContext*);

    public:
        output(uint32_t status,
               uint32_t task_error,
               uint32_t sys_errnum,
               uint32_t elapsed_time) :
            m_status(status),
            m_task_error(task_error),
            m_sys_errnum(sys_errnum),
            m_elapsed_time(elapsed_time) {}

        uint32_t
        status() const {
            return m_status;
        }

        void
        set_retval(uint32_t status) {
            m_status = status;
        }

        uint32_t 
        task_error() const {
            return m_task_error;
        }

        void 
        set_task_error(uint32_t task_error) {
            m_task_error = task_error;
        }

        uint32_t 
        sys_errnum() const {
            return m_sys_errnum;
        }

        uint32_t
        elapsed_time() const {
            return m_elapsed_time;
        }

        void 
        set_sys_errnum(uint32_t errnum) {
            m_sys_errnum = errnum;
        }

        explicit 
        output(const hermes::detail::remote_transfer_out_t& out) {
            m_status = out.status;
            m_task_error = out.task_error;
            m_sys_errnum = out.sys_errnum;
            m_elapsed_time = out.elapsed_time;
        }

        explicit 
        operator hermes::detail::remote_transfer_out_t() {
            return {m_status, m_task_error, m_sys_errnum, m_elapsed_time};
        }

    private:
        uint32_t m_status;
        uint32_t m_task_error;
        uint32_t m_sys_errnum;
        uint32_t m_elapsed_time;
    };
};

struct resource_stat {
    // forward declarations of public input/output types for this RPC
    class input;
    class output;

    // traits used so that the engine knows what to do with the RPC
    using self_type = resource_stat;
    using handle_type = hermes::rpc_handle<self_type>;
    using input_type = input;
    using output_type = output;
    using mercury_input_type = hermes::detail::resource_stat_in_t;
    using mercury_output_type = hermes::detail::resource_stat_out_t;

    // RPC public identifier
    constexpr static const uint16_t public_id = 44;

    // RPC internal Mercury identifier
    constexpr static const uint16_t mercury_id = public_id;

    // RPC name
    constexpr static const auto name = "resource_stat";

    // requires response?
    constexpr static const auto requires_response = true;

    // Mercury callback to serialize input arguments
    constexpr static const auto mercury_in_proc_cb = 
        HG_GEN_PROC_NAME(resource_stat_in_t);

    // Mercury callback to serialize output arguments
    constexpr static const auto mercury_out_proc_cb = 
        HG_GEN_PROC_NAME(resource_stat_out_t);

    class input {

        template <typename ExecutionContext>
        friend hg_return_t hermes::detail::post_to_mercury(ExecutionContext*);

    public:
        input(const std::string& address,
              const std::string& nsid,
              uint32_t resource_type,
              const std::string& resource_name) :
            m_address(address),
            m_nsid(nsid),
            m_resource_type(resource_type),
            m_resource_name(resource_name) {

#ifdef HERMES_DEBUG_BUILD
            this->print("this", __PRETTY_FUNCTION__);
#endif

        }

#ifdef HERMES_DEBUG_BUILD
        input(input&& rhs) :
            m_address(std::move(rhs.m_address)),
            m_nsid(std::move(rhs.m_nsid)),
            m_resource_type(std::move(rhs.m_resource_type)),
            m_resource_name(std::move(rhs.m_resource_name)) {

            rhs.m_resource_type = 0;

            this->print("this", __PRETTY_FUNCTION__);
            rhs.print("rhs", __PRETTY_FUNCTION__);
        }

        input(const input& other) :
            m_address(other.m_address),
            m_nsid(other.m_nsid),
            m_resource_type(other.m_resource_type),
            m_resource_name(other.m_resource_name) {

            this->print("this", __PRETTY_FUNCTION__);
            other.print("other", __PRETTY_FUNCTION__);
        }

        input& 
        operator=(input&& rhs) {

            if(this != &rhs) {
                m_address = std::move(rhs.m_address);
                m_nsid = std::move(rhs.m_nsid);
                m_resource_type = std::move(rhs.m_resource_type);
                m_resource_name = std::move(rhs.m_resource_name);

                rhs.m_resource_type = 0;
            }

            this->print("this", __PRETTY_FUNCTION__);
            rhs.print("rhs", __PRETTY_FUNCTION__);

            return *this;
        }

        input& 
        operator=(const input& other) {
            
            if(this != &other) {
                m_address = other.m_address;
                m_nsid = other.m_nsid;
                m_resource_type = other.m_resource_type;
                m_resource_name = other.m_resource_name;
            }

            this->print("this", __PRETTY_FUNCTION__);
            other.print("other", __PRETTY_FUNCTION__);

            return *this;
        }
#else // HERMES_DEBUG_BUILD
        input(input&& rhs) = default;
        input(const input& other) = default;
        input& operator=(input&& rhs) = default;
        input& operator=(const input& other) = default;
#endif // ! HERMES_DEBUG_BUILD

        std::string
        address() const {
            return m_address;
        }

        std::string
        nsid() const {
            return m_nsid;
        }

        uint32_t
        resource_type() const {
            return m_resource_type;
        }

        std::string
        resource_name() const {
            return m_resource_name;
        }

#ifdef HERMES_DEBUG_BUILD
        void
        print(const std::string& id,
              const std::string& caller = "") const {

            (void) id;
            auto c = caller.empty() ? "unknown_caller" : caller;

            HERMES_DEBUG2("{}, {} ({}) = {{", caller, id, fmt::ptr(this));
            HERMES_DEBUG2("  m_address: \"{}\" ({} -> {}),", 
                         m_address, fmt::ptr(&m_address), 
                         fmt::ptr(m_address.c_str()));
            HERMES_DEBUG2("  m_nsid: \"{}\" ({} -> {}),", 
                         m_nsid, fmt::ptr(&m_nsid),
                         fmt::ptr(m_nsid.c_str()));
            HERMES_DEBUG2("  m_resource_type: {},", 
                         m_resource_type); 
            HERMES_DEBUG2("  m_resource_name: \"{}\" ({} -> {}),", 
                         m_resource_name, fmt::ptr(&m_resource_name),
                         fmt::ptr(m_resource_name.c_str()));
            HERMES_DEBUG2("}}");
        }
#endif // ! HERMES_DEBUG_BUILD

//TODO: make private
        explicit
        input(const hermes::detail::resource_stat_in_t& other) :
            m_address(other.address),
            m_nsid(other.nsid),
            m_resource_type(other.resource_type),
            m_resource_name(other.resource_name) { 

            HERMES_DEBUG("input::input(const hermes::detail::resource_stat_in_t&){{");
            HERMES_DEBUG("  m_address: {} ({}),", 
                         m_address, fmt::ptr(&m_address));
            HERMES_DEBUG("  m_nsid: {} ({}),", 
                         m_nsid, fmt::ptr(&m_nsid));
            HERMES_DEBUG("  m_resource_type: {},", 
                         m_resource_type); 
            HERMES_DEBUG("  m_resource_name: \"{}\" ({} -> {}),", 
                         m_resource_name, fmt::ptr(&m_resource_name),
                         fmt::ptr(m_resource_name.c_str()));
            HERMES_DEBUG("}}");
        }
        
        explicit
        operator hermes::detail::resource_stat_in_t() {
            return {m_address.c_str(),
                    m_nsid.c_str(), 
                    m_resource_type, 
                    m_resource_name.c_str()};
        }


    private:
        std::string m_address;
        std::string m_nsid;
        uint32_t m_resource_type;
        std::string m_resource_name;
    };

    class output {

        template <typename ExecutionContext>
        friend hg_return_t hermes::detail::post_to_mercury(ExecutionContext*);

    public:
        output(uint32_t return_value,
               uint64_t packed_size) :
            m_return_value(return_value),
            m_packed_size(packed_size) {}

        uint32_t
        return_value() const {
            return m_return_value;
        }

        uint64_t
        packed_size() const {
            return m_packed_size;
        }

        explicit 
        output(const hermes::detail::resource_stat_out_t& out) {
            m_return_value = out.return_value;
            m_packed_size = out.packed_size;
        }

        explicit 
        operator hermes::detail::resource_stat_out_t() {
            return {m_return_value, m_packed_size};
        }

    private:
        uint32_t m_return_value;
        uint64_t m_packed_size;
    };
};

} // namespace rpc
} // namespace norns

#undef HG_GEN_PROC_NAME

#endif // __NORNS_IO_TRANSFERORS_RPCS_HPP__
