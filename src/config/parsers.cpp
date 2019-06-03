#include <cstdint>
#include <string>
#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "utils.hpp"
#include "parsers.hpp"

namespace bfs = boost::filesystem;

namespace norns {
namespace config {
namespace parsers {

bool parse_bool(const std::string& name, const std::string& value) {

    if(value == "1" || boost::algorithm::to_lower_copy(value) == "true") {
        return true;
    }

    if(value == "0" || boost::algorithm::to_lower_copy(value) == "false") {
        return false;
    }

    throw std::invalid_argument("Value provided for option '" + name + "' is not boolean");
}

uint32_t parse_number(const std::string& name, const std::string& value) {

    int32_t optval = 0;

    try {
        optval = std::stoi(value);
    } catch(...) {
        throw std::invalid_argument("Value provided for option '" + name + "' is not a number");
    }

    if(optval <= 0) {
        throw std::invalid_argument("Value provided for option '" + name + "' must be greater than zero");
    }

    return static_cast<uint32_t>(optval);
}

bfs::path parse_path(const std::string& name, const std::string& value) {

    (void) name;

    return bfs::path(value);
}

bfs::path parse_existing_path(const std::string& name, const std::string& value) {

    if(!bfs::exists(value)) {
        throw std::invalid_argument("Path '" + value + "' in option '" + name + "' does not exist");
    }

    return bfs::path(value);
}

uint64_t parse_capacity(const std::string& name, const std::string& value) {

    try {
        return utils::parse_size(value);
    }
    catch(const std::exception& e) {
        throw std::invalid_argument("Value provided in option '" + name + "' is invalid");
    }
}

} // namespace parsers
} // namespace config
} // namespace norns
