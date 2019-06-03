#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include <algorithm>
#include <iterator>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "utils.hpp"
#include "compare-files.hpp"
#include <iostream>

namespace bfs = boost::filesystem;

bool compare(const std::vector<int>& data, const bfs::path& file) {

    char buffer[4096];

    int fd = ::open(file.c_str(), O_RDONLY);

    if(fd == -1) {
        return false;
    }

    struct stat stbuf;

    if(fstat(fd, &stbuf) == -1) {
        return false;
    }

    if(stbuf.st_size != static_cast<off_t>(data.size() * sizeof(int))) {
        return false;
    }

    const char* pdata = reinterpret_cast<const char*>(data.data());

    while(true) {

        ssize_t nr = read(fd, buffer, sizeof(buffer));

        if(nr <= 0) {
            break;
        }

        if(memcmp(pdata, buffer, nr) != 0) {
            return false;
        }

        pdata += nr;
    }

    close(fd);

    return true;
}

bool compare_files(const bfs::path& filename1, const bfs::path& filename2) {

    bfs::ifstream file1(filename1, std::ifstream::ate | std::ifstream::binary);

    if(!file1) {
        throw std::invalid_argument("could not open " + filename1.string());
    }

    bfs::ifstream file2(filename2, std::ifstream::ate | std::ifstream::binary);

    if(!file2) {
        throw std::invalid_argument("could not open " + filename2.string());
    }

    if(file1.tellg() != file2.tellg()) {
        return false;
    }

    file1.seekg(0);
    file2.seekg(0);

    std::istreambuf_iterator<char> begin1(file1);
    std::istreambuf_iterator<char> begin2(file2);
    std::istreambuf_iterator<char> end;

    return range_equal(begin1, end, begin2, end);
}

bool compare_directories(const bfs::path& dirname1, const bfs::path& dirname2) {

    bfs::recursive_directory_iterator it(dirname1);
    bfs::recursive_directory_iterator end;

    for(; it != end; ++it) {
        boost::system::error_code ec;
        const bfs::path pdst = bfs::canonical(dirname2 / bfs::relative(*it, dirname1), ec);

        if(ec) {
            return false;
        }

        if(bfs::is_directory(*it) && bfs::is_directory(pdst)) {
			if(!compare_directories(*it, pdst)) {
				return false;
			}
			continue;
        }
        else if(bfs::is_directory(pdst)){
            return false;
        }

        if(!compare_files(*it, pdst)) {
            return false;
        }
    }

    return true;
}

