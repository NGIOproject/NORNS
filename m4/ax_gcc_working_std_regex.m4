# SYNOPSIS
#
#   AX_GCC_WORKING_STD_REGEX
#
# DESCRIPTION
#
#   This macro checks if GCC has a working implementation of std::regex, since
#   GCC 4.7/4.8 provided only valid headers and function stubs. Further 
#   information is available:
#   <https://stackoverflow.com/questions/12530406/is-gcc-4-8-or-earlier-buggy-about-regular-expressions>.
#
#   If the std::regex implementation is functional, the macro will set the 
#   variable 'gcc_has_working_std_regex' to 'yes' and AC_DEFINE the C preprocessor
#   variable HAVE_WORKING_STD_REGEX.
#
#   An automake conditional can be subsequently defined as
#       AM_CONDITIONAL([HAVE_WORKING_STD_REGEX], 
#                      [test x$gcc_has_working_std_regex = xyes])
#
# LICENSE
#
#   Copyright (c) 2019 Alberto Miranda <alberto.miranda@bsc.es>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 10

AC_DEFUN([AX_GCC_WORKING_STD_REGEX],
[
    gcc_has_working_std_regex=no
    AC_CACHE_CHECK(
        [whether GCC has a working std::regex implementation],
        ax_cv_gcc_working_regex,
        [
            AC_LANG_PUSH([C++])
            AC_COMPILE_IFELSE([
                AC_LANG_PROGRAM(
                        [[ @%:@include <regex> ]],
                        [[
                           @%:@if __cplusplus >= 201103L &&                          \
                               (!defined(__GLIBCXX__) || (__cplusplus >= 201402L) || \
                                   (defined(_GLIBCXX_REGEX_DFS_QUANTIFIERS_LIMIT) || \
                                   defined(_GLIBCXX_REGEX_STATE_LIMIT)            || \
                                       (defined(_GLIBCXX_RELEASE)                 && \
                                       _GLIBCXX_RELEASE > 4)))
                           @%:@define HAVE_WORKING_REGEX 1
                           @%:@else
                           @%:@define HAVE_WORKING_REGEX 0
                           @%:@error std::regex does not work
                           @%:@endif
                        ]]
                )],
                ax_cv_gcc_working_regex=yes,
                ax_cv_gcc_working_regex=no)
            AC_LANG_POP([C++])
        ]
    )

    if test "x$ax_cv_gcc_working_regex" = "xyes"; then
        gcc_has_working_std_regex=yes
        AC_DEFINE([HAVE_WORKING_STD_REGEX], [1],
                  [define if the std::regex implementation works as expected])
    fi
])
