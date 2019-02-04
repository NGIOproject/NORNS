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
        ((uint32_t) (backend_type))
        ((uint32_t) (resource_type))
        ((hg_const_string_t) (resource_name))
        ((hg_bulk_t) (buffers)))

MERCURY_GEN_PROC(remote_transfer_out_t,
        ((int32_t) (retval)))

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
              const std::string& resource_name,
              const hermes::exposed_memory& buffers) :
            m_address(address),
            m_in_nsid(in_nsid),
            m_out_nsid(out_nsid),
            m_backend_type(backend_type),
            m_resource_type(resource_type),
            m_resource_name(resource_name),
            m_buffers(buffers) { }

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

        std::string
        resource_name() const {
            return m_resource_name;
        }

        hermes::exposed_memory
        buffers() const {
            return m_buffers;
        }

//TODO: make private
        explicit
        input(const hermes::detail::remote_transfer_in_t& other) :
            m_address(other.address),
            m_in_nsid(other.in_nsid),
            m_out_nsid(other.out_nsid),
            m_backend_type(other.backend_type),
            m_resource_type(other.resource_type),
            m_resource_name(other.resource_name),
            m_buffers(other.buffers) { }
        
        explicit
        operator hermes::detail::remote_transfer_in_t() {
            return {m_address.c_str(),
                    m_in_nsid.c_str(), 
                    m_out_nsid.c_str(), 
                    m_backend_type,
                    m_resource_type, 
                    m_resource_name.c_str(), 
                    hg_bulk_t(m_buffers)};
        }


    private:
        std::string m_address;
        std::string m_in_nsid;
        std::string m_out_nsid;
        uint32_t m_backend_type;
        uint32_t m_resource_type;
        std::string m_resource_name;
        hermes::exposed_memory m_buffers;
    };

    class output {

        template <typename ExecutionContext>
        friend hg_return_t hermes::detail::post_to_mercury(ExecutionContext*);

    public:
        output(int32_t retval) :
            m_retval(retval) { }

        int32_t
        retval() const {
            return m_retval;
        }

        void
        set_retval(int32_t retval) {
            m_retval = retval;
        }

        explicit 
        output(const hermes::detail::remote_transfer_out_t& out) {
            m_retval = out.retval;
        }

        explicit 
        operator hermes::detail::remote_transfer_out_t() {
            return {m_retval};
        }

    private:
        int32_t m_retval;
    };
};

} // namespace rpc
} // namespace norns

#undef HG_GEN_PROC_NAME

#endif // __NORNS_IO_TRANSFERORS_RPCS_HPP__
