ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib examples

DIST_SUBDIRS = lib examples

bin_PROGRAMS = src/dloom #app

src_dloom_SOURCES = \
	src/dloom.cpp

src_dloom_CXXFLAGS = 			\
	@TBB_CFLAGS@ 				\
	-std=gnu++11 -Wall -Wextra

src_dloom_LDFLAGS = \
	@TBB_LIBS@

#norn_a_AR = $(CXX) -ar -o 

#app_SOURCES = app.c norn.c
#app_CFLAGS = 
