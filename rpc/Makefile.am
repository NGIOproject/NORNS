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
