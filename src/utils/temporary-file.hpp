#ifndef NORNS_UTILS_TEMP_FILE_HPP
#define NORNS_UTILS_TEMP_FILE_HPP

#include <boost/filesystem.hpp>
#include <system_error>

namespace bfs = boost::filesystem;

namespace norns {
namespace utils {

struct temporary_file {

    temporary_file() noexcept;

    // create an empty temporary file from pattern at parent_dir
    temporary_file(const std::string& pattern,
                   const bfs::path& parent_dir,
                   std::error_code& ec) noexcept;

    // create a temporary file of size 'prealloc_size' from pattern at parent_dir
    temporary_file(const std::string& pattern,
                   const bfs::path& parent_dir,
                   std::size_t prealloc_size,
                   std::error_code& ec) noexcept;

    // initialize temporary file from an already existing file
    temporary_file(const bfs::path& filename,
                   std::error_code& ec) noexcept;

    temporary_file(const temporary_file& other) = delete;

    temporary_file(temporary_file&& rhs) = default;

    temporary_file& operator=(const temporary_file& other) = delete;

    temporary_file& operator=(temporary_file&& rhs) = default;

    ~temporary_file();

    bfs::path 
    path() const noexcept;

    void
    reserve(std::size_t size, 
            std::error_code& ec) const noexcept;

    void
    manage(const bfs::path& filename,
           std::error_code& ec) noexcept;

    bfs::path
    release() noexcept;

    bfs::path m_filename;
};

} // namespace utils
} // namespace norns

#endif // NORNS_UTILS_TEMP_FILE_HPP
