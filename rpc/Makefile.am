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

#lib_LTLIBRARIES = libnorns-rpc.la

#AM_CPPFLAGS = \
#	-I$(builddir)

#libnorns_rpc_la_SOURCES = 	\
#	norns-rpc.h \
#	messages.pb-c.c \
#	messages.pb-c.h

#BUILT_SOURCES = \
#	messages.pb-c.c \
#	messages.pb-c.h	\
#	messages.pb.cc \
#	messages.pb.h

#MOSTLYCLEANFILES = \
#	messages.pb-c.c \
#	messages.pb-c.h \
#	messages.pb.cc \
#	messages.pb.h

#libnorns_rpc_la_CFLAGS = \
#	-Wall -Wextra
#
#libnorns_rpc_la_LDFLAGS = \
#	-version-info 0:1:0

#libnorns_rpc_la_LIBADD = \
#	@PROTOBUF_C_LIBS@


#%.pb-c.c %.pb-c.h: %.proto
#	$(PROTOC_C) --proto_path=$(srcdir) --c_out=$(builddir) $^
#
#%.pb.cc %.pb.h: %.proto
#	$(PROTOC) --proto_path=$(srcdir) --cpp_out=$(builddir) $^

dist_noinst_DATA = messages.proto
