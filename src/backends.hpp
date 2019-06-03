#ifndef __NORNS_BACKENDS_HPP__
#define  __NORNS_BACKENDS_HPP__

#include <memory>
#include "backends/backend-base.hpp"
#include "backends/process-memory.hpp"
#include "backends/remote-backend.hpp"
#include "backends/lustre-fs.hpp"
#include "backends/posix-fs.hpp"
#include "backends/nvml-dax.hpp"

namespace norns {
namespace storage {

    constexpr static const auto memory_nsid = "[[internal::memory]]";

    static const auto process_memory_backend = std::make_shared<detail::process_memory>(memory_nsid);
//    static const auto remote_backend = std::make_shared<detail::remote_backend>();
} // namespace storage
} // namespace norns

#endif /* __NORNS_BACKENDS_HPP__ */

