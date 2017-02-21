#                       -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

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

# check that Intel's TBB
PKG_CHECK_MODULES([TBB], [tbb])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_CONFIG_FILES([
        Makefile
        lib/Makefile
        examples/Makefile])
AC_OUTPUT
