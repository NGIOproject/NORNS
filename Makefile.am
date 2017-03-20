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

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib examples

DIST_SUBDIRS = lib examples

bin_PROGRAMS = src/urd

src_urd_SOURCES = 			\
	src/main.cpp			\
	src/ipc-listener.hpp	\
	src/urd.cpp				\
	src/urd.hpp

src_urd_CXXFLAGS = 				\
	@TBB_CFLAGS@ 				\
	@LIBEV_CFLAGS@				\
	-std=gnu++11 -Wall -Wextra

src_urd_CPPFLAGS = 				\
	-I$(top_srcdir)/include

src_urd_LDFLAGS = \
	@TBB_LIBS@	  \
	@LIBEV_LIBS@  \
	-pthread

src_urd_LDADD = 	\
	-lboost_system
