#ifndef __COMPARE_FILES_HPP__
#define __COMPARE_FILES_HPP__

#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

template<typename InputIterator1, typename InputIterator2>
bool range_equal(InputIterator1 first1, InputIterator1 last1,
                 InputIterator2 first2, InputIterator2 last2) {

    while(first1 != last1 && first2 != last2) {
        if(*first1 != *first2) {
            return false;
        }
        ++first1;
        ++first2;
    }
    return (first1 == last1) && (first2 == last2);
}

bool compare(const std::vector<int>& data, const bfs::path& file);
bool compare_files(const bfs::path& filename1, const bfs::path& filename2);
bool compare_directories(const bfs::path& dirname1, const bfs::path& dirname2);

#endif /* __COMPARE_FILES_HPP__ */
