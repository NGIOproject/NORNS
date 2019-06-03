#include "config.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/system/error_code.hpp>

#include "temporary-file.hpp"
#include "file-handle.hpp"
#include "logger.hpp"

namespace {

using norns::utils::file_handle;

void
reallocate(const file_handle& fh, 
           std::size_t size,
           std::error_code& ec) noexcept {

    if(!fh) {
        ec.assign(EINVAL, std::generic_category());
        return;
    }

#ifdef HAVE_FALLOCATE
    if(::fallocate(fh.native(), 0, 0, size) == -1) {
        if(errno != EOPNOTSUPP) {
            ec.assign(errno, std::generic_category());
            return;
        }
#endif // HAVE_FALLOCATE

        // filesystem doesn't support fallocate(), 
        // fallback to truncate()
        if(::ftruncate(fh.native(), size) != 0) {
            ec.assign(errno, std::generic_category());
            return;
        }

#ifdef HAVE_FALLOCATE
    }
#endif // HAVE_FALLOCATE
}

} // anonymous namespace

namespace norns {
namespace utils {

temporary_file::temporary_file() noexcept { }

temporary_file::temporary_file(const std::string& pattern,
                               const bfs::path& parent_dir,
                               std::error_code& ec) noexcept :
    temporary_file(pattern, parent_dir, 0, ec) {}

temporary_file::temporary_file(const std::string& pattern,
                               const bfs::path& parent_dir,
                               std::size_t reserve_size,
                               std::error_code& ec) noexcept {

    boost::system::error_code bec;

    const auto filename = parent_dir / bfs::unique_path(pattern, bec);

    if(bec) {
        // LOGGER_ERROR("Failed to create unique path from pattern: {}", 
        //              bec.message());
        ec.assign(bec.value(), std::generic_category());
        return;
    }

    if(!bfs::exists(parent_dir, bec)) {
        // LOGGER_ERROR("parent_dir does not exist: {}", bec.message());
        ec.assign(ENOENT, std::generic_category());
        return;
    }

    if(bec) {
        // LOGGER_ERROR("parent_dir does not exist: {}", bec.message());
        ec.assign(bec.value(), std::generic_category());
        return;
    }

    file_handle fh(
        ::open(filename.c_str(), 
               O_CREAT | O_WRONLY | O_EXCL,
               S_IRUSR | S_IWUSR));

    if(!fh) {
        ec.assign(errno, std::generic_category());
        LOGGER_ERROR("Failed to create temporary file {}: {}", 
                     filename, ec.message());
        return;
    }

    if(reserve_size != 0) {
        ::reallocate(fh, reserve_size, ec);
    }

    m_filename = filename;
}

temporary_file::temporary_file(const bfs::path& filename, 
                               std::error_code& ec) noexcept {
    this->manage(filename, ec);
}

temporary_file::~temporary_file() {

    if(m_filename.empty()) {
        return;
    }

    LOGGER_DEBUG("{} Removing temporary file {}",
                 __PRETTY_FUNCTION__, m_filename);

    boost::system::error_code bec;
    bfs::remove(m_filename, bec);

    if(bec) {
        LOGGER_ERROR("Failed to remove temporary_file {}: {}", 
                     m_filename, bec.message());
    }
}

bfs::path
temporary_file::path() const noexcept {
    return m_filename;
}

void
temporary_file::reserve(std::size_t size, 
                        std::error_code& ec) const noexcept {

    file_handle fh{::open(m_filename.c_str(), O_WRONLY)};

    if(!fh) {
        ec.assign(errno, std::generic_category());
        // LOGGER_ERROR("Failed to allocate space for file: {}", ec.message());
        return;
    }

    ::reallocate(fh, size, ec);
}

void
temporary_file::manage(const bfs::path& filename,
                       std::error_code& ec) noexcept {

    boost::system::error_code bec;
    const auto existing_filename = bfs::canonical(filename, bec);

    if(bec) {
        // LOGGER_ERROR("Failed to get canonical path from filename: {}", 
        //              bec.message());
        ec.assign(bec.value(), std::generic_category());
        return;
    }

    m_filename = existing_filename;
}

bfs::path
temporary_file::release() noexcept {
    const bfs::path ret{std::move(m_filename)};
    m_filename.clear();
    return ret;
}

} // namespace utils
} // namespace norns
