lib_LTLIBRARIES = libnorn.la

include_HEADERS = $(top_srcdir)/include/libnorn.h

AM_CPPFLAGS = -I$(top_srcdir)/include

libnorn_la_SOURCES = 	\
	norn.c				\
	$(top_srcdir)/include/libnorn.h

libnorn_la_CFLAGS = \
	-Wall -Wextra

libnorn_la_LDFLAGS = -version-info 0:1:0
