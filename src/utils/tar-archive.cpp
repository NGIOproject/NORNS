/************************************************************************* 
 * Copyright (C) 2017-2019 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of NORNS, a service that allows other programs to   *
 * start, track and manage asynchronous transfers of data resources      *
 * between different storage backends.                                   *
 *                                                                       *
 * See AUTHORS file in the top level directory for information regarding *
 * developers and contributors.                                          *
 *                                                                       *
 * This software was developed as part of the EC H2020 funded project    *
 * NEXTGenIO (Project ID: 671951).                                       *
 *     www.nextgenio.eu                                                  *
 *                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining *
 * a copy of this software and associated documentation files (the       *
 * "Software"), to deal in the Software without restriction, including   *
 * without limitation the rights to use, copy, modify, merge, publish,   *
 * distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to *
 * the following conditions:                                             *
 *                                                                       *
 * The above copyright notice and this permission notice shall be        *
 * included in all copies or substantial portions of the Software.       *
 *                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                 *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS   *
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN    *
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN     *
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE      *
 * SOFTWARE.                                                             *
 *************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <archive.h>
#include <archive_entry.h>

#include "utils.hpp"
#include "file-handle.hpp"
#include "logger.hpp"
#include "tar-archive.hpp"

// typedefs for convenience
using archive_ptr = std::unique_ptr<struct archive, 
                                    decltype(&archive_write_free)>;
using entry_ptr = std::unique_ptr<struct archive_entry, 
                                  decltype(&::archive_entry_free)>;

// helper functions
namespace {

constexpr std::size_t
align(std::size_t n, std::size_t block_size) {
    return n & ~(block_size - 1);
}

constexpr std::size_t
xalign(std::size_t n, std::size_t block_size) {
    return (n - align(n, block_size)) != 0 ? 
            align(n, block_size) + block_size :
            n;
}

std::error_code
append_file_data(struct archive* ar,
                 const bfs::path& name, 
                 std::size_t size) {

    using norns::utils::file_handle;
    std::error_code ec;

    file_handle fh(::open(name.c_str(), O_RDONLY));

    if(!fh) {
        ec.assign(errno, std::generic_category());
        return ec;
    }

    std::array<char, 16384> buffer;
    off_t offset = 0;

	while(offset < static_cast<off_t>(size)) {
		ssize_t n = ::pread(fh.native(), buffer.data(), buffer.size(), offset);

		switch(n) {
            case 0:
                return ec;

            case -1:
                if(errno == EINTR) {
                    continue;
                }

                ec.assign(errno, std::generic_category());
                return ec;

            default:
            {
                ssize_t m = archive_write_data(ar, buffer.data(), n);

                if(m == -1) {
                    ec.assign(::archive_errno(ar), std::generic_category());
                    return ec;
                }

                // FIXME: we are assuming that we can successfully append
                // the whole buffer to the archive in one call to 
                // archive_write_data()
                assert(m == n);
                offset += n;
            }
        }
	}

	return ec;
}

std::error_code
append_file(struct archive* arc,
            const bfs::path& source_path,
            const bfs::path& archive_path,
            const entry_ptr& entry) {

    std::error_code ec;

    // set attributes and write the header for the file
    struct stat stbuf;
    if(::stat(source_path.c_str(), &stbuf) != 0) {
        ec.assign(errno, std::generic_category()); 
        return ec;
    }

    ::archive_entry_set_filetype(entry.get(), AE_IFREG);
    ::archive_entry_copy_sourcepath(entry.get(), source_path.c_str());
    ::archive_entry_copy_pathname(entry.get(), archive_path.c_str());
    ::archive_entry_copy_stat(entry.get(), &stbuf);

    if(::archive_write_header(arc, entry.get()) != ARCHIVE_OK) {
        ec.assign(::archive_errno(arc), std::generic_category()); 
        return ec;
    }

    // append the actual file data to the archive
    return ::append_file_data(arc, source_path, stbuf.st_size);
}

std::error_code
append_directory_header(struct archive* arc,
                        const bfs::path& source_path,
                        const bfs::path& archive_path,
                        const entry_ptr& entry) {
    std::error_code ec;

    // set attributes and write the header for the file
    struct stat stbuf;
    if(::stat(source_path.c_str(), &stbuf) != 0) {
        ec.assign(errno, std::generic_category()); 
        return ec;
    }

    ::archive_entry_set_filetype(entry.get(), AE_IFDIR);
    ::archive_entry_copy_sourcepath(entry.get(), source_path.c_str());
    ::archive_entry_copy_pathname(entry.get(), archive_path.c_str());
    ::archive_entry_copy_stat(entry.get(), &stbuf);

    if(::archive_write_header(arc, entry.get()) != ARCHIVE_OK) {
        ec.assign(::archive_errno(arc), std::generic_category()); 
        return ec;
    }

    return ec;
}

// replace 'real_parent' with 'archive_parent' in 'source' also 
// adding a leading '/' if 'archive_parent' is empty 
bfs::path
transform(const bfs::path& source, 
          const bfs::path& real_parent,
          const bfs::path& archive_parent) {

    using norns::utils::lexical_normalize;
    using norns::utils::remove_leading_separator;

    assert(source.is_absolute());
    assert(real_parent.is_absolute());

    const bfs::path norm_source = lexical_normalize(source);
    const bfs::path norm_real_parent = lexical_normalize(real_parent);
    const bfs::path norm_archive_parent = lexical_normalize(archive_parent);

    assert(norm_source.is_absolute());
    assert(norm_real_parent.is_absolute());

    return (norm_archive_parent.empty() ? 
                bfs::path("/") : 
                norm_archive_parent) / 
            bfs::relative(norm_source, norm_real_parent);
}

} // anonymous namespace

namespace norns {
namespace utils {

tar::tar(const bfs::path& filename, 
         openmode op,
         std::error_code& ec) :
    m_path(filename),
    m_openmode(op) {

    if(filename.empty()) {
        ec.assign(EINVAL, std::generic_category());
        return;
    }

    ec.assign(0, std::generic_category());

    if(op == openmode::create) {
        archive_ptr arc(::archive_write_new(), ::archive_write_free);

        if(!arc) {
            ec.assign(errno, std::generic_category());
            LOGGER_ERROR("Failed to open archive for writing: {}", 
                         ec.message());
            return;
        }

        if(::archive_write_set_format_ustar(arc.get()) == ARCHIVE_FATAL) {
            ec.assign(::archive_errno(arc.get()), std::generic_category());
            LOGGER_ERROR("Failed to set output format to USTAR: {}", 
                         ::archive_error_string(arc.get()));
            return;
        }

        if(::archive_write_open_filename(arc.get(), filename.c_str()) 
                == ARCHIVE_FATAL) {
            ec.assign(::archive_errno(arc.get()), std::generic_category());
            LOGGER_ERROR("Failed to open archive for writing: {}", 
                         ::archive_error_string(arc.get()));
            return;
        }

        m_archive = arc.release();
    }
    else if(op == openmode::open) {

        archive_ptr arc(::archive_read_new(), ::archive_read_free);

        if(!arc) {
            ec.assign(errno, std::generic_category());
            LOGGER_ERROR("Failed to open archive for reading: {}", 
                         ec.message());
            return;
        }

        if(::archive_read_support_format_tar(arc.get()) == ARCHIVE_FATAL) {
            ec.assign(::archive_errno(arc.get()), std::generic_category());
            LOGGER_ERROR("Failed to determine archive format: {}",
                         ::archive_error_string(arc.get()));
            return;
        }

        if(::archive_read_open_filename(arc.get(), filename.c_str(), 16384) 
                == ARCHIVE_FATAL) {
            ec.assign(::archive_errno(arc.get()), std::generic_category());
            LOGGER_ERROR("Failed to open archive for reading: {}", 
                         ::archive_error_string(arc.get()));
            return;
        }

        m_archive = arc.release();
    }
    else {
        ec.assign(EINVAL, std::generic_category());
        return;
    }
}

tar::~tar() {
    this->release();
}

void
tar::add_file(const bfs::path& source_file, 
              const bfs::path& archive_file,
              std::error_code& ec) {

    if(m_archive == nullptr || m_openmode != tar::create ||
       source_file.empty()) {
        ec.assign(EINVAL, std::generic_category()); 
        return;
    }

    ec.assign(0, std::generic_category());

    boost::system::error_code bec;
    const bfs::path source_path = bfs::canonical(source_file, bec);

    if(bec) {
        ec.assign(bec.value(), std::generic_category());
        return;
    }

    if(!bfs::is_regular(source_path))  {
        ec.assign(EINVAL, std::generic_category()); 
        return;
    }

    entry_ptr entry(::archive_entry_new2(m_archive), ::archive_entry_free);

    if(!entry) {
        ec.assign(::archive_errno(m_archive), std::generic_category()); 
        return;
    }

    ec = ::append_file(m_archive, source_path, archive_file, entry);
}

void
tar::add_directory(const bfs::path& source_dir, 
                   const bfs::path& archive_dir,
                   std::error_code& ec) {

//    fmt::print(stderr, "::add_directory({}, alias={})\n", 
//               source_dir, archive_dir);

    if(m_archive == nullptr || m_openmode != tar::create || 
       source_dir.empty()) {
        ec.assign(EINVAL, std::generic_category()); 
        return;
    }

    ec.assign(0, std::generic_category());

    boost::system::error_code bec;
    const bfs::path source_path = bfs::canonical(source_dir, bec);

    if(bec) {
        ec.assign(bec.value(), std::generic_category());
        return;
    }

    if(!bfs::is_directory(source_path))  {
        ec.assign(EINVAL, std::generic_category()); 
        return;
    }

    entry_ptr entry(::archive_entry_new2(m_archive), ::archive_entry_free);

    if(!entry) {
        ec.assign(::archive_errno(m_archive), std::generic_category()); 
        return;
    }

#if 0
    // if the transformed_path for the source directory is empty, it means
    // that the user specified a transformation where SOURCE_PATH had to be 
    // replaced either with "" or with "/". In any of those cases,
    // we avoid creating a header for SOURCE_PATH since the directory will not
    // be included into the archive.
    const bfs::path transformed_path = 
        ::transform(source_path, source_path, archive_dir);

    if(!transformed_path.empty()) {
//        fmt::print(stderr, "    ::append_directory_header({}, tp: {})\n", 
//                   source_path, transformed_path);
        ec = ::append_directory_header(m_archive, source_path, 
                                       transformed_path, entry);
        if(ec) {
            return;
        }
    }
#else
    const bfs::path transformed_path = 
        ::transform(source_path, source_path, archive_dir);

//        fmt::print(stderr, "    ::append_directory_header({}, tp: {})\n", 
//                   source_path, transformed_path);
    ec = ::append_directory_header(m_archive, source_path, 
                                   transformed_path, entry);
    if(ec) {
        return;
    }
#endif

    // directory_iterator
    bfs::recursive_directory_iterator it(source_path);
    bfs::recursive_directory_iterator end;

    for(bfs::recursive_directory_iterator it(source_path, bec);
        it != bfs::recursive_directory_iterator();
        ++it) {

        if(bec) {
            ec.assign(bec.value(), std::generic_category());
            return;
        }

//        fmt::print(stderr, "  processing:  {}\n", *it);
        ::archive_entry_clear(entry.get());
        const bfs::path transformed_path = 
            ::transform(*it, source_path, archive_dir);

        if(bfs::is_regular(*it)) {
//            fmt::print(stderr, "    ::append_file({}, tp: {})\n", 
//                       *it, transformed_path);
            ec = ::append_file(m_archive, *it, transformed_path, entry);

            if(ec) {
                return;
            }
        }
        else if(bfs::is_directory(*it)) {
//            fmt::print(stderr, "    ::append_directory_header({}, tp: {})\n", 
//                       *it, transformed_path);
            ec = ::append_directory_header(m_archive, *it, transformed_path,
                                           entry);

            if(ec) {
                return;
            }
        }
        else {
            /* FIXME ignored */
            LOGGER_WARN("Found unhandled file type when adding "
                        "directory: {}", *it);
            continue;
        }
    }
}

void
tar::release() {

    if(m_archive == nullptr) {
        return;
    }

    if(m_openmode == tar::create) {
        if(::archive_write_close(m_archive) == ARCHIVE_FATAL) {
            LOGGER_ERROR("Failed to close TAR archive: {}",
                         ::archive_error_string(m_archive));
            // intentionally fall through
        }

        if(::archive_write_free(m_archive) == ARCHIVE_FATAL) {
            LOGGER_ERROR("Failed to release TAR archive resources: {}",
                            ::archive_error_string(m_archive));
        }
    }
    else {
        assert(m_openmode == tar::open);

        if(::archive_read_close(m_archive) == ARCHIVE_FATAL) {
            LOGGER_ERROR("Failed to close TAR archive: {}",
                         ::archive_error_string(m_archive));
            // intentionally fall through
        }

        if(::archive_read_free(m_archive) == ARCHIVE_FATAL) {
            LOGGER_ERROR("Failed to release TAR archive resources: {}",
                            ::archive_error_string(m_archive));
        }
    }

    m_archive = nullptr;
}

void
tar::extract(const bfs::path& parent_dir, 
             std::error_code& ec) {

    if(m_archive == nullptr || m_openmode != tar::open || 
       parent_dir.empty() || !bfs::is_directory(parent_dir)) {
        ec.assign(EINVAL, std::generic_category());
        return;
    }

    boost::system::error_code bec;
    const bfs::path dest_path = bfs::canonical(parent_dir, bec);

    if(bec) {
        ec.assign(bec.value(), std::generic_category());
        return;
    }

    ec.assign(0, std::generic_category());

    // Select which attributes we want to restore
    int flags = 
        // full permissions (including SGID, SUID, and sticky bits) should be
        // restored exactly as specified
        ARCHIVE_EXTRACT_PERM |
        // refuse to extract any object whose final location would be altered
        // by a symlink on disk
        ARCHIVE_EXTRACT_SECURE_SYMLINKS |
        // refuse to extract a path that contains a .. element 
        // anywhere within it
        ARCHIVE_EXTRACT_SECURE_NODOTDOT;

    struct archive_entry* entry;

    for(;;) {

        int rv = ::archive_read_next_header(m_archive, &entry);

        if(rv == ARCHIVE_EOF) {
            break;
        }

        if(rv != ARCHIVE_OK) {
            ec.assign(::archive_errno(m_archive), std::generic_category());
            LOGGER_ERROR("Failed to read next header entry: {}",
                         ::archive_error_string(m_archive));
            return;
        }

        const char* p = ::archive_entry_pathname(entry);

        if(!p) {
            ec.assign(ENOMEM, std::generic_category());
            LOGGER_ERROR("Failed to retrieve entry pathname: {}", ec.message());
            return;
        }

        const bfs::path archive_pathname(p);
        const bfs::path new_pathname(parent_dir / 
                remove_leading_separator(archive_pathname));

//        fmt::print("found entry: {}\n", archive_pathname);
//        fmt::print("  new path: {}\n", new_pathname);

        ::archive_entry_copy_pathname(entry, new_pathname.c_str());

        if(::archive_read_extract(m_archive, entry, flags) != ARCHIVE_OK) {
            ec.assign(::archive_errno(m_archive), std::generic_category());
            LOGGER_ERROR("Failed to extract archive entry {} to {}: {}",
                         archive_pathname, new_pathname,
                         ::archive_error_string(m_archive));
            return;
        }
    }
}

bfs::path
tar::path() const {
    return m_path;
}

std::size_t
tar::estimate_size_once_packed(const bfs::path& source_path,
                               /*const bfs::path& packed_path,*/
                               std::error_code& ec) {

    std::size_t sz = 0;
    boost::system::error_code error;

    if(bfs::is_directory(source_path)) {
        for(bfs::recursive_directory_iterator it(source_path, error);
            it != bfs::recursive_directory_iterator();
            ++it) {

            if(error) {
                LOGGER_ERROR("Failed to traverse path {}", source_path);
                ec = std::make_error_code(
                        static_cast<std::errc>(error.value()));
                return 0;
            }

            if(bfs::is_directory(*it)) {
                sz += TAR_BLOCK_SIZE;
            }
            else if(bfs::is_regular(*it) || bfs::is_symlink(*it)) {
                sz += TAR_BLOCK_SIZE + 
                    ::xalign(bfs::file_size(*it, error), TAR_BLOCK_SIZE);

                if(error) {
                    LOGGER_ERROR("Failed to determine size for {}", *it);
                    ec = std::make_error_code(
                            static_cast<std::errc>(error.value()));
                    return 0;
                }
            }
            else {
                // not a critical error, report it and go on
                LOGGER_ERROR("Unhandled file type at {}", *it);
            }
        }

//        if(!packed_path.empty() && packed_path == "/") {
            // we need to take into account the header for the basedir
            // 'source_path' since recursive_directory_iterator skips it
            sz += TAR_BLOCK_SIZE;
//        }
    }
    else {
        sz += TAR_BLOCK_SIZE + 
            ::xalign(bfs::file_size(source_path), TAR_BLOCK_SIZE);
    }

    // EOF
    sz += 2*TAR_BLOCK_SIZE;

    return sz;
}

} // namespace utils
} // namespace norns
