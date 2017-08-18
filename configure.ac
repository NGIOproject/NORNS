# Copyright (C) 2017 Barcelona Supercomputing Center
#                    Centro Nacional de Supercomputacion
#
# This file is part of the Data Scheduler, a daemon for tracking and managing
# requests for asynchronous data transfer in a hierarchical storage environment.
#
# See AUTHORS file in the top level directory for information
# regarding developers and contributors.
#
# The Data Scheduler is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Data Scheduler is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Data Scheduler.  If not, see <http://www.gnu.org/licenses/>.
#

AC_INIT([Norn-s loom], [1.0], [arnau.bagocastro at bsc.es])
AC_PREREQ([2.69])
AC_CONFIG_SRCDIR([src/config.h.in])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([src/config.h])

AM_INIT_AUTOMAKE([1.9 foreign subdir-objects])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC

LT_INIT

# check that c++11 is supported by the compiler
AX_CHECK_COMPILE_FLAG([-std=c++11], [CXXFLAGS+=" -std=c++11"], [
    echo "C++ compiler cannot compile C++11 code"
    exit -1
])

# Checks for libraries.I


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

# check for some BOOST libraries
AX_BOOST_BASE([1.53],, [
    AC_MSG_ERROR([
        This software requires BOOST >= 1.53, but it was not found in your system
    ])
])
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_ASIO
AX_BOOST_PROGRAM_OPTIONS


# check that Intel's TBB is available. some distros don't include
PKG_CHECK_MODULES([TBB], [tbb],
                  [],
AC_MSG_ERROR([This software requires the development files of Intel TBB.
 * In Ubuntu/Debian: apt-get install libtbb-dev
 * In RHEL/CentOS: yum install tbb-devel])
)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CONFIG_FILES([
        Makefile
        etc/Makefile
        lib/Makefile
        examples/Makefile])
AC_OUTPUT
