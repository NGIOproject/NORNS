bin_PROGRAMS = dloom app

dloom_SOURCES = dloom.cpp
#dloom_CXXFLAGS = $(CXXFLAGS) -I/usr/local/ -ltbb -std=gnu++11 -L tbb2017_20161128oss/lib/intel64/gcc4.7/
dloom_CXXFLAGS = -I/usr/local/ -std=gnu++11
dloom_LDFLAGS = -L/usr/local
dloom_LDADD = -ltbb
norn_a_AR = $(CXX) -ar -o 

app_SOURCES = app.c norn.c
#app_CFLAGS = 
