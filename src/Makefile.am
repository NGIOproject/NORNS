##########################################################################
#  Copyright (C) 2017-2018 Barcelona Supercomputing Center               #
#                          Centro Nacional de Supercomputacion           #
#  All rights reserved.                                                  #
#                                                                        #
#  This file is part of the NORNS Data Scheduler, a service that allows  #
#  other programs to start, track and manage asynchronous transfers of   #
#  data resources transfers requests between different storage backends. #
#                                                                        #
#  See AUTHORS file in the top level directory for information           #
#  regarding developers and contributors.                                #
#                                                                        #
#  The NORNS Data Scheduler is free software: you can redistribute it    #
#  and/or modify it under the terms of the GNU Lesser General Public     #
#  License as published by the Free Software Foundation, either          #
#  version 3 of the License, or (at your option) any later version.      #
#                                                                        #
#  The NORNS Data Scheduler is distributed in the hope that it will be   #
#  useful, but WITHOUT ANY WARRANTY; without even the implied warranty   #
#  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  #
#  Lesser General Public License for more details.                       #
#                                                                        #
#  You should have received a copy of the GNU Lesser General             #
#  Public License along with the NORNS Data Scheduler.  If not, see      #
#  <http://www.gnu.org/licenses/>.                                       #
##########################################################################

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = urd

noinst_LTLIBRARIES = \
	liburd_resources.la \
	liburd_aux.la

################################################################################
# build 'liburdresources_aux.la', a convenience library with all the code
# related to data resources
################################################################################
liburd_resources_la_SOURCES = \
	resources/local_posix_path/local-path.hpp \
	resources/local_posix_path/detail/local-path-impl.cpp \
	resources/local_posix_path/detail/local-path-impl.hpp \
	resources/local_posix_path/detail/local-path-info.cpp \
	resources/local_posix_path/detail/local-path-info.hpp \
	resources/memory_buffer/memory-buffer.hpp \
	resources/memory_buffer/detail/memory-region-impl.cpp \
	resources/memory_buffer/detail/memory-region-impl.hpp \
	resources/memory_buffer/detail/memory-region-info.cpp \
	resources/memory_buffer/detail/memory-region-info.hpp \
	resources/remote_posix_path/remote-path.hpp \
	resources/remote_posix_path/detail/remote-path-impl.cpp \
	resources/remote_posix_path/detail/remote-path-impl.hpp \
	resources/remote_posix_path/detail/remote-path-info.cpp \
	resources/remote_posix_path/detail/remote-path-info.hpp \
	resources/resource.hpp \
	resources/resource-info.hpp \
	resources/resource-type.hpp \
	resources/shared_posix_path/shared-path.hpp \
	resources/shared_posix_path/detail/shared-path-impl.cpp \
	resources/shared_posix_path/detail/shared-path-impl.hpp \
	resources/shared_posix_path/detail/shared-path-info.cpp \
	resources/shared_posix_path/detail/shared-path-info.hpp

liburd_resources_la_CXXFLAGS = \
	-std=gnu++11 -Wall -Wextra

liburd_resources_la_CPPFLAGS = \
	-DSPDLOG_ENABLE_SYSLOG \
	@BOOST_CPPFLAGS@ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/resources \
	-I$(top_srcdir)/rpc	\
	-I$(top_builddir)/rpc

liburd_resources_la_LDFLAGS = \
    @BOOST_ASIO_LIB@ \
    @BOOST_FILESYSTEM_LIB@ \
	@BOOST_LDFLAGS@	\
    @BOOST_PROGRAM_OPTIONS_LIB@	\
    @BOOST_SYSTEM_LIB@ \
    @BOOST_THREAD_LIB@ \
	@PROTOBUF_LIBS@ \
	-pthread


################################################################################
# build 'liburd_aux.la', a convenience library so that we can build both the 
# daemon and any tests that may need it
################################################################################
MOSTLYCLEANFILES = \
	defaults.cpp \
	messages.pb.cc \
	messages.pb.h

liburd_aux_la_SOURCES = \
	$(top_srcdir)/rpc/norns-rpc.h \
	api.hpp \
	api/dispatch-table.hpp \
	api/listener.hpp \
	api/message.hpp \
	api/request.hpp \
	api/request.cpp \
	api/response.hpp \
	api/response.cpp \
	backends.hpp \
	backends/backend-base.cpp \
	backends/backend-base.hpp \
	backends/lustre-fs.cpp \
	backends/lustre-fs.hpp \
	backends/nvml-dax.cpp \
	backends/nvml-dax.hpp \
	backends/posix-fs.cpp \
	backends/posix-fs.hpp \
	backends/process-memory.cpp \
	backends/process-memory.hpp \
	backends/remote-backend.cpp \
	backends/remote-backend.hpp \
	common.hpp \
	common/make-unique.hpp \
	common/types.cpp \
	common/types.hpp \
	common/unique-ptr-cast.hpp \
	defaults.hpp \
	io.hpp \
	io/fake-task.cpp \
	io/fake-task.hpp \
	io/task.cpp \
	io/task.hpp \
	io/task-manager.cpp \
	io/task-manager.hpp \
	io/task-stats.cpp \
	io/task-stats.hpp \
	io/transferors.hpp \
	io/transferors/local-path-to-local-path.cpp \
	io/transferors/local-path-to-local-path.hpp \
	io/transferors/local-path-to-shared-path.cpp \
	io/transferors/local-path-to-shared-path.hpp \
	io/transferors/local-path-to-remote-path.cpp \
	io/transferors/local-path-to-remote-path.hpp \
	io/transferors/memory-to-local-path.cpp \
	io/transferors/memory-to-local-path.hpp \
	io/transferors/memory-to-shared-path.cpp \
	io/transferors/memory-to-shared-path.hpp \
	io/transferors/memory-to-remote-path.cpp \
	io/transferors/memory-to-remote-path.hpp \
	io/transferor-registry.cpp \
	io/transferor-registry.hpp \
	job.hpp \
	logger.hpp \
	resources.hpp \
	settings.cpp \
	settings.hpp \
	signal-listener.hpp \
	thread-pool.hpp \
	thread-pool/thread-pool.hpp \
	thread-pool/thread-pool-queue.hpp \
	urd.cpp	\
	urd.hpp	\
	utils.cpp \
	utils.hpp

nodist_liburd_aux_la_SOURCES = \
	defaults.cpp \
	messages.pb.cc \
	messages.pb.h

liburd_aux_la_CXXFLAGS = \
	@TBB_CFLAGS@ \
	-std=gnu++11 -Wall -Wextra

liburd_aux_la_CPPFLAGS = \
	-DSPDLOG_ENABLE_SYSLOG \
	@BOOST_CPPFLAGS@ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/resources \
	-I$(top_srcdir)/rpc	\
	-I$(top_builddir)/rpc

liburd_aux_la_LDFLAGS = \
    @BOOST_ASIO_LIB@ \
    @BOOST_FILESYSTEM_LIB@ \
	@BOOST_LDFLAGS@	\
    @BOOST_PROGRAM_OPTIONS_LIB@	\
    @BOOST_SYSTEM_LIB@ \
    @BOOST_THREAD_LIB@ \
	@PROTOBUF_LIBS@ \
	liburd_resources.la \
	-pthread

BUILT_SOURCES = \
	defaults.cpp \
	messages.pb.cc \
	messages.pb.h

defaults.cpp: Makefile
	@( echo "/* This file autogenerated by Makefile */"; \
	   echo "#include <thread>"; \
	   echo "#include \"defaults.hpp\""; \
	   echo ""; \
	   echo "namespace norns {"; \
	   echo "namespace defaults {"; \
	   echo "    const char* progname           = \"urd\";"; \
	   echo "    const bool  daemonize          = true;"; \
	   echo "    const bool  use_syslog         = false;"; \
	   echo ""; \
\
	   echo "    const bool dry_run             = false;"; \
\
	   echo "    const char* global_socket      = \"$(localstatedir)/urd/global.socket.2\";"; \
	   echo "    const char* control_socket     = \"$(localstatedir)/urd/control.socket.2\";"; \
	   echo "    const in_port_t remote_port    = 42000;"; \
	   echo "    const char* pidfile            = \"$(localstatedir)/urd/urd.pid\";"; \
\
	   echo "    const uint32_t workers_in_pool = std::thread::hardware_concurrency();"; \
	   echo "    const char* config_file        = \"$(sysconfdir)/norns.conf\";"; \
	   echo "} // namespace defaults"; \
	   echo "} // namespace norns"; \
	 ) > $@

%.pb.cc %.pb.h: $(top_srcdir)/rpc/%.proto
	$(PROTOC) --proto_path=$(top_srcdir)/rpc --cpp_out=$(builddir) $^

EXTRA_liburd_aux_la_DEPENDENCIES = \
	liburd_resources.la

################################################################################
# build 'urd', the main binary for the service daemon
################################################################################
urd_SOURCES = \
	main.cpp

urd_CXXFLAGS = \
	@TBB_CFLAGS@ \
	-std=gnu++11 -Wall -Wextra

urd_CPPFLAGS = \
	-DSPDLOG_ENABLE_SYSLOG \
	@BOOST_CPPFLAGS@ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/rpc	\
	-I$(top_builddir)/rpc

urd_LDFLAGS = \
    @BOOST_ASIO_LIB@ \
    @BOOST_FILESYSTEM_LIB@ \
	@BOOST_LDFLAGS@	\
    @BOOST_PROGRAM_OPTIONS_LIB@	\
    @BOOST_SYSTEM_LIB@ \
    @BOOST_THREAD_LIB@ \
	@PROTOBUF_LIBS@ \
	liburd_aux.la

# we also need to include it as an additional dependency, since automake
# will no longer detect it as a dependency
EXTRA_urd_DEPENDENCIES = \
	liburd_aux.la
