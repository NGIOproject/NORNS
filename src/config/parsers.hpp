#ifndef __PARSERS_HPP__
#define __PARSERS_HPP__

#include <cstdint>
#include <string>
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

namespace norns {
namespace config {
namespace parsers {

bool parse_bool(const std::string& name, const std::string& value);
uint32_t parse_number(const std::string& name, const std::string& value);
bfs::path parse_path(const std::string& name, const std::string& value);
bfs::path parse_existing_path(const std::string& name, const std::string& value);
uint64_t parse_capacity(const std::string& name, const std::string& value);

} // namespace parsers
} // namespace config
} // namespace norns

#endif /* __PARSERS_HPP__ */
