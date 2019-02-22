//#include "common.hpp"
#include <hermes.hpp>
#include "rpcs.hpp"

namespace hermes { namespace detail {

//==============================================================================
// register request types so that they can be used by users and the engine
//
void
register_user_request_types() {
    (void) registered_requests().add<norns::rpc::remote_transfer>();
    (void) registered_requests().add<norns::rpc::resource_stat>();
}

}} // namespace hermes::detail
