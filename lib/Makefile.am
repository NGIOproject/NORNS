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

#dist_noinst_DATA = messages.proto
#
#EXTRA_DIST = messages.proto

MOSTLYCLEANFILES = \
	messages.pb-c.c \
	messages.pb-c.h

lib_LTLIBRARIES = \
	libnorns.la

include_HEADERS = \
	$(top_srcdir)/include/norns.h
	$(top_srcdir)/include/norns/norns_error.h

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/rpc

libnorns_la_SOURCES = 	\
	norns.c				\
	errors.c            \
	$(top_srcdir)/include/norns.h \
	$(top_srcdir)/rpc/norns-rpc.h \
	messages.pb-c.c \
	messages.pb-c.h

libnorns_la_CFLAGS = \
	-Wall -Wextra -std=gnu99

libnorns_la_LDFLAGS = \
	-version-info 0:1:0

libnorns_la_LIBADD = \
	@PROTOBUF_C_LIBS@

BUILT_SOURCES = \
	messages.pb-c.c \
	messages.pb-c.h

%.pb-c.c %.pb-c.h: $(top_srcdir)/rpc/%.proto
	$(PROTOC_C) --proto_path=$(top_srcdir)/rpc --c_out=$(builddir) $^
