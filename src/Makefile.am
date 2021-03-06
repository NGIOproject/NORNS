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
	resources/remote_resource/remote-resource.hpp \
	resources/remote_resource/detail/remote-resource-impl.cpp \
	resources/remote_resource/detail/remote-resource-impl.hpp \
	resources/remote_resource/detail/remote-resource-info.cpp \
	resources/remote_resource/detail/remote-resource-info.hpp \
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
	-std=gnu++11 -Wall -Wextra -fdevirtualize

liburd_resources_la_CPPFLAGS = \
	-DSPDLOG_ENABLE_SYSLOG \
	-DHERMES_DISABLE_INTERNAL_MAKE_UNIQUE \
	@BOOST_CPPFLAGS@ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/externals/hermes/include \
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
    @MERCURY_LIBS@ \
	@PROTOBUF_LIBS@ \
	@LIBARCHIVE_LIBS@ \
	-pthread


################################################################################
# build 'liburd_aux.la', a convenience library so that we can build both the 
# daemon and any tests that may need it
################################################################################
MOSTLYCLEANFILES = \
	config/defaults.cpp \
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
	auth/process-credentials.cpp \
	auth/process-credentials.hpp \
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
	config.hpp \
	config/settings.cpp \
	config/settings.hpp \
	config/config-schema.hpp \
	config/file-options.hpp \
	config/file_options/file-schema.hpp \
	config/file_options/options-map.hpp \
	config/file_options/yaml-parser.hpp \
	config/keywords.hpp \
	config/parsers.cpp \
	config/parsers.hpp \
	config/defaults.hpp \
	context.hpp \
	io.hpp \
	io/task.hpp \
	io/task-copy.hpp \
	io/task-info.cpp \
	io/task-info.hpp \
	io/task-manager.cpp \
	io/task-manager.hpp \
	io/task-move.hpp \
	io/task-noop.hpp \
	io/task-remote-transfer.hpp \
	io/task-remove.hpp \
	io/task-stats.cpp \
	io/task-stats.hpp \
	io/transferors.hpp \
	io/transferors/transferor.hpp \
	io/transferors/local-path-to-local-path.cpp \
	io/transferors/local-path-to-local-path.hpp \
	io/transferors/local-path-to-shared-path.cpp \
	io/transferors/local-path-to-shared-path.hpp \
	io/transferors/local-path-to-remote-resource.cpp \
	io/transferors/local-path-to-remote-resource.hpp \
	io/transferors/remote-resource-to-local-path.cpp \
	io/transferors/remote-resource-to-local-path.hpp \
	io/transferors/memory-to-local-path.cpp \
	io/transferors/memory-to-local-path.hpp \
	io/transferors/memory-to-shared-path.cpp \
	io/transferors/memory-to-shared-path.hpp \
	io/transferors/memory-to-remote-path.cpp \
	io/transferors/memory-to-remote-path.hpp \
	io/transferors/memory-to-remote-resource.cpp \
	io/transferors/memory-to-remote-resource.hpp \
	io/transferor-registry.cpp \
	io/transferor-registry.hpp \
	job.hpp \
	logger.hpp \
	resources.hpp \
	rpcs.cpp \
	rpcs.hpp \
	signal-listener.hpp \
	thread-pool.hpp \
	thread-pool/thread-pool.hpp \
	thread-pool/thread-pool-queue.hpp \
	urd.cpp	\
	urd.hpp	\
	utils.cpp \
	utils.hpp \
	utils/file-handle.hpp \
	utils/tar-archive.cpp \
	utils/tar-archive.hpp \
	utils/temporary-file.hpp \
	utils/temporary-file.cpp

nodist_liburd_aux_la_SOURCES = \
	config/defaults.cpp \
	messages.pb.cc \
	messages.pb.h

liburd_aux_la_CXXFLAGS = \
	-std=gnu++11 -Wall -Wextra -fdevirtualize

liburd_aux_la_CPPFLAGS = \
	-DSPDLOG_ENABLE_SYSLOG \
	-DHERMES_DISABLE_INTERNAL_MAKE_UNIQUE \
	@BOOST_CPPFLAGS@ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/externals/hermes/include \
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
    @MERCURY_LIBS@ \
	@PROTOBUF_LIBS@ \
	@YAMLCPP_LIBS@ \
	@LIBARCHIVE_LIBS@ \
	liburd_resources.la \
	-pthread

BUILT_SOURCES = \
	config/defaults.cpp \
	messages.pb.cc \
	messages.pb.h

config/defaults.cpp: Makefile
	@( echo "/* This file autogenerated by Makefile */"; \
	   echo "#include <thread>"; \
	   echo "#include \"config/defaults.hpp\""; \
	   echo ""; \
	   echo "namespace norns {"; \
	   echo "namespace config {"; \
	   echo "namespace defaults {"; \
	   echo "    const char* progname             = \"urd\";"; \
	   echo "    const bool daemonize             = true;"; \
	   echo "    const bool use_syslog            = false;"; \
	   echo "    const bool use_console           = false;"; \
	   echo "    const bfs::path log_file         = boost::filesystem::path();"; \
	   echo "    const uint32_t log_file_max_size = static_cast<uint32_t>(16*1024*1024);"; \
	   echo ""; \
\
	   echo "    const bool dry_run               = false;"; \
	   echo "    const uint32_t dry_run_duration  = 100;"; \
\
	   echo "    const char* global_socket        = \"$(localstatedir)/global.socket.2\";"; \
	   echo "    const char* control_socket       = \"$(localstatedir)/control.socket.2\";"; \
	   echo "    const char* bind_address         = \"127.0.0.1\";"; \
	   echo "    const in_port_t remote_port      = 42000;"; \
	   echo "    const char* pidfile              = \"$(localstatedir)/urd.pid\";"; \
\
	   echo "    const uint32_t workers_in_pool   = std::thread::hardware_concurrency();"; \
	   echo "    const char* staging_directory    = \"/tmp/urd/\";"; \
	   echo "    const uint32_t backlog_size      = 128;"; \
	   echo "    const char* config_file          = \"$(sysconfdir)/norns.conf\";"; \
	   echo "} // namespace defaults"; \
	   echo "} // namespace config"; \
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
	-std=gnu++11 -Wall -Wextra -fdevirtualize

urd_CPPFLAGS = \
	-DSPDLOG_ENABLE_SYSLOG \
	-DHERMES_DISABLE_INTERNAL_MAKE_UNIQUE \
	@BOOST_CPPFLAGS@ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/src/externals/hermes/include \
	-I$(top_srcdir)/rpc	\
	-I$(top_builddir)/rpc

urd_LDFLAGS = \
    @BOOST_ASIO_LIB@ \
    @BOOST_FILESYSTEM_LIB@ \
	@BOOST_LDFLAGS@	\
    @BOOST_PROGRAM_OPTIONS_LIB@	\
    @BOOST_SYSTEM_LIB@ \
    @BOOST_THREAD_LIB@ \
    @MERCURY_LIBS@ \
	@PROTOBUF_LIBS@ \
	liburd_aux.la

# we also need to include it as an additional dependency, since automake
# will no longer detect it as a dependency
EXTRA_urd_DEPENDENCIES = \
	liburd_aux.la

urd-install-exec-hook:
	-setcap cap_sys_ptrace,cap_chown=+ep $(DESTDIR)$(bindir)/urd



################################################################################
# Hooks
################################################################################

install-exec-hook: urd-install-exec-hook
