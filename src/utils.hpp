#ifndef NORNS_UTILS_HPP
#define NORNS_UTILS_HPP

#include <boost/version.hpp>
#include <boost/filesystem.hpp>
#include <system_error>
#include <sstream>
#include <iomanip>
#include <cstdint>

#include "common.hpp"
#include "utils/tar-archive.hpp"
#include "utils/temporary-file.hpp"

namespace norns {
namespace utils {

uint64_t parse_size(const std::string& str);

template <typename T> 
std::string n2hexstr(T i, bool zero_pad=false) {
    std::stringstream ss;

    if(zero_pad) {
       ss << std::setfill('0') << std::setw(sizeof(T) << 1);
    }

    ss << std::showbase << std::hex << i;
    return ss.str();
}

boost::filesystem::path lexical_normalize(const boost::filesystem::path& pathname,
                                          bool as_directory=false);

boost::filesystem::path
remove_trailing_separator(const boost::filesystem::path& pathname);

boost::filesystem::path
remove_leading_separator(const boost::filesystem::path& pathname);

} // namespace utils
} // namespace norns

#if BOOST_VERSION <= 106000 // 1.6.0

#include <boost/filesystem.hpp>

namespace boost { namespace filesystem {

path relative(path from_path, path to_path);

}} // namespace boost::filesystem

#endif

#endif // NORNS_UTILS_HPP
