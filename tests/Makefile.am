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

AM_CPPFLAGS = -I m4

TESTS = api 

check_PROGRAMS = $(TESTS) api_interactive

END = 

api_CXXFLAGS = \
	-Wall -Wextra \
	$(END)

api_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/rpc \
	-I$(top_srcdir)/src \
	$(END)

api_SOURCES = \
	catch.hpp \
	api-main.cpp \
	api-backend-register.cpp \
	api-backend-unregister.cpp \
	api-copy-local-data.cpp \
	api-job-register.cpp \
	api-job-update.cpp \
	api-job-unregister.cpp \
	api-ping.cpp \
	api-process-add.cpp \
	api-process-remove.cpp \
	api-resource-init.cpp \
	api-task-init.cpp \
	api-task-submit.cpp \
	api-task-status.cpp \
	fake-daemon.cpp \
	fake-daemon.hpp \
	$(END)

# NOTE: we cannot pass 'liburd_aux.la' directly to 'urd_LDFLAGS' as usual,
# because backends rely on the initialization of several static (and 
# unreferenced) variables to self-register, so that they can be automatically 
# constructed. Since they are not referenced elsewhere, the linker removes
# the variables, which makes the self-registering process not work. To avoid 
# this, we need to link the convenience library with '-Wl,--whole-archive'
# to force the inclusion of these symbols and then disable it with 
# '-Wl,--no-whole-archive'. Unfortunately, the order of flags
# cannot be controlled, which forces us to define it explicitly as
# '-Wl,--whole-archive,.libs/liburd_aux.a,--no-whole-archive' and to include
# several extra dependencies which would not be needed otherwise
api_LDFLAGS = \
	@TBB_LIBS@ \
    @BOOST_ASIO_LIB@ \
	@BOOST_LDFLAGS@	\
	@BOOST_FILESYSTEM_LIB@ \
    @BOOST_PROGRAM_OPTIONS_LIB@	\
    @BOOST_SYSTEM_LIB@ \
    @BOOST_THREAD_LIB@ \
	@PROTOBUF_LIBS@ \
	-Wl,--whole-archive,$(top_builddir)/src/.libs/liburd_aux.a,--no-whole-archive \
	$(top_builddir)/lib/libnorns.la \
	$(END)

EXTRA_api_DEPENDENCIES = \
	$(top_builddir)/src/liburd_aux.la \
	$(top_builddir)/lib/libnorns.la \
	$(END)

# The following produces a version of the api tests where the urd daemon 
# is not started automatically by the test, so that a running daemon can be
# tested interactively
api_interactive_CXXFLAGS = $(api_CXXFLAGS)
api_interactive_CPPFLAGS = $(api_CPPFLAGS) -DUSE_REAL_DAEMON
api_interactive_SOURCES = $(api_SOURCES)
api_interactive_LDFLAGS = $(api_LDFLAGS)
EXTRA_api_interactive_DEPENDENCIES = $(EXTRA_api_DEPENDENCIES)
