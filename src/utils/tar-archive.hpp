#ifndef NORNS_UTILS_TAR_ARCHIVE_HPP
#define NORNS_UTILS_TAR_ARCHIVE_HPP

#include <boost/filesystem.hpp>
#include <system_error>

// forward declare 'struct archive'
struct archive;

namespace bfs = boost::filesystem;

namespace norns {
namespace utils {

struct tar {

    enum class openmode : int {
        create = 0,
        open   = 1,
    };

    constexpr static const auto TAR_BLOCK_SIZE = 512;
    constexpr static const openmode create = openmode::create;
    constexpr static const openmode open = openmode::open;

    tar(const bfs::path& filename, openmode op, std::error_code& ec);

    ~tar();

    void
    add_file(const bfs::path& real_name, 
             const bfs::path& archive_name,
             std::error_code& ec);

    void
    add_directory(const bfs::path& real_dir, 
                  const bfs::path& archive_dir,
                  std::error_code& ec);

    void
    release();

    void
    extract(const bfs::path& parent_dir,
            std::error_code& ec);

    bfs::path
    path() const;

    static std::size_t
    estimate_size_once_packed(const bfs::path& source_path,
                              /*const bfs::path& packed_path,*/
                              std::error_code& ec);

    struct archive* m_archive = nullptr;
    bfs::path m_path;
    openmode m_openmode;
};

} // namespace utils
} // namespace norns

#endif // NORNS_UTILS_TAR_ARCHIVE_HPP
