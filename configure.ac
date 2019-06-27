##########################################################################
#  Copyright (C) 2017-2019 Barcelona Supercomputing Center               #
#                          Centro Nacional de Supercomputacion           #
#  All rights reserved.                                                  #
#                                                                        #
#  This file is part of NORNS, a service that allows other programs to   #
#  start, track and manage asynchronous transfers of data resources      #
#  between different storage backends.                                   #
#                                                                        #
#  See AUTHORS file in the top level directory for information regarding #
#  developers and contributors.                                          #
#                                                                        #
#  This software was developed as part of the EC H2020 funded project    #
#  NEXTGenIO (Project ID: 671951).                                       #
#      www.nextgenio.eu                                                  #
#                                                                        #
#  Permission is hereby granted, free of charge, to any person obtaining #
#  a copy of this software and associated documentation files (the       #
#  "Software"), to deal in the Software without restriction, including   #
#  without limitation the rights to use, copy, modify, merge, publish,   #
#  distribute, sublicense, and/or sell copies of the Software, and to    #
#  permit persons to whom the Software is furnished to do so, subject to #
#  the following conditions:                                             #
#                                                                        #
#  The above copyright notice and this permission notice shall be        #
#  included in all copies or substantial portions of the Software.       #
#                                                                        #
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       #
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    #
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.#
#  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  #
#  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  #
#  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     #
#  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                #
##########################################################################

AC_INIT(m4_esyscmd_s([awk -F ':[ \t]+' '$1 ~ /^ *Name$/ {gsub(" ", "", $2); print $2; exit}' ./META ]), 
        m4_esyscmd_s([awk -F ':[ \t]+' '$1 ~ /^ *Version$/ {gsub(" ", "", $2); print $2; exit}' ./META ]), 
        m4_esyscmd_s([awk -F ':[ \t]+' '$1 ~ /^ *Bugs$/ {gsub(" ", "", $2); print $2; exit}' ./META ]), 
        [],
        [],
        )
X_AC_NORNS_META
AC_PREREQ([2.69])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.9 foreign subdir-objects])

AC_LANG([C])
AC_LANG([C++])

# Checks for programs.
AC_PROG_AWK
AC_PROG_SED
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP

AC_PATH_PROG([FILE], [file])
AS_IF([test "x${FILE}" == "x"],
      [AC_MSG_ERROR([The file utility was not found in \$PATH. Please install it and rerun.])])

LT_INIT

# check that c++11 is supported by the compiler
AX_CHECK_COMPILE_FLAG([-std=c++11], [CXXFLAGS+=" -std=c++11"], [
    echo "C++ compiler cannot compile C++11 code"
    exit -1
])

# check for pkg-config
PKG_PROG_PKG_CONFIG

if test -z "$PKG_CONFIG";
then
    AC_MSG_ERROR([Could not find pkg-config.
 * You may not have pkg-config installed
    - In Ubuntu/Debian: apt-get install pkg-config
    - In RHEL/Fedora: yum install pkgconfig
    - In SuSE: zypper install pkg-config
    - Install from sources at http://www.freedesktop.org/wiki/Software/pkg-config/
 * If it is installed but not in the PATH then you may want to set the PKG_CONFIG variable])
fi

################################################################################
### check whether tests should be built
################################################################################
is_enabled_build_tests="no"

AC_ARG_ENABLE([tests],
    AS_HELP_STRING([--enable-tests], [Build norns self tests]), 
    [
        if test x$enableval = xyes -o x$enableval = x;
        then
            is_enabled_build_tests="yes"
        else if test x$enableval = xno -o x$enableval = x;
            then
                is_enabled_build_tests="no"
            else
                AC_MSG_ERROR([This option can only be given 'yes' or 'no' values])
            fi
        fi
    ],
    [ is_enabled_build_tests="no" ])

AM_CONDITIONAL([BUILD_TESTS], test x$is_enabled_build_tests = xyes)


################################################################################
### check for dependencies
################################################################################

# check for base BOOST libraries
AX_BOOST_BASE([1.53],, [
    AC_MSG_ERROR([
        This software requires BOOST >= 1.53, but it was not found in your system
    ])
])
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_ASIO
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_THREAD

# check whether the compiler has a functional std::regex
AX_GCC_WORKING_STD_REGEX
AS_IF([test "x$gcc_has_working_std_regex" = xno],
      [
        # fallback to boost::regex
        AC_MSG_WARN([falling back to boost::regex because std::regex is not functional])
        AX_BOOST_REGEX 
      ], [])
AM_CONDITIONAL([HAVE_WORKING_STD_REGEX], 
               test x$GCC_HAS_WORKING_STD_REGEX = xyes)

# only check for std::regex/boost::regex if we are building tests
AS_IF([test "x$is_enabled_build_tests" = "xyes"], 
      [ AC_CONFIG_FILES(tests/Makefile) ], [])

# check for mercury
PKG_CHECK_MODULES([MERCURY], [mercury >= 0.26])

# check for libyaml-cpp
PKG_CHECK_MODULES([YAMLCPP], [yaml-cpp >= 0.5.1])

# check for protobuf-c
PKG_CHECK_MODULES([PROTOBUF_C], [libprotobuf-c >= 1.0.0])

AC_CHECK_PROG([PROTOC_C], [protoc-c], [protoc-c])
AS_IF([test "x${PROTOC_C}" == "x"],
          [AC_MSG_ERROR([ProtoBuf C compiler "protoc-c" not found.])])

# check for protobuf
PKG_CHECK_MODULES([PROTOBUF], [protobuf >= 2.5.0])

AC_CHECK_PROG([PROTOC], [protoc], [protoc])
AS_IF([test "x${PROTOC}" == "x"],
          [AC_MSG_ERROR([ProtoBuf compiler "protoc" not found.])])


# check for libyaml (manually since it doesn't provide a pkgconfig file)
AC_SEARCH_LIBS([yaml_parser_initialize], [yaml], 
               [YAML_LIBS="-lyaml"
                AC_SUBST(YAML_LIBS)], 
               [AC_MSG_ERROR([This software requires libyaml >= 0.1.4])])

# check for libarchive
PKG_CHECK_MODULES([LIBARCHIVE], [libarchive >= 3.1.2])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNC([fallocate],[fallocate],[fallocate])
AS_IF([test "x${PROTOC}" == "x"],
          [AC_MSG_ERROR([ProtoBuf compiler "protoc" not found.])])

AC_CHECK_FUNC([fallocate], 
              [AC_DEFINE([HAVE_FALLOCATE], 
                         [1], [Define if file preallocation is available])])

################################################################################
### write makefiles
################################################################################
AC_CONFIG_FILES([
        Makefile
        doc/Makefile
        etc/Makefile
        examples/Makefile
        include/Makefile
        lib/Makefile
        src/Makefile
])
AC_OUTPUT
