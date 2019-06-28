# NORNS
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://travis-ci.com/bsc-ssrg/NORNS.svg?branch=master)](https://travis-ci.com/bsc-ssrg/NORNS)
[![Coverage Status](https://codecov.io/gh/bsc-ssrg/NORNS/branch/master/graph/badge.svg)](https://codecov.io/gh/bsc-ssrg/NORNS)

NORNS is an open-source data scheduling service that orchestrates asynchronous
data transfers between different storage backends in an HPC cluster. Through
its API, NORNS provides three different functions. First, it allows system
administrators to expose the storage architecture of an HPC cluster by creating
**dataspaces** associated to different storage backends such as node-local NVMs 
and POSIX file systems, or system-wide parallel file systems and object stores, 
thus making them available to applications, services, and users. 
Second, it provides a framework for submitting and monitoring the asynchronous
transfers of **data resources** between the (local and remote) dataspaces
available to a user, such as process buffers, POSIX files and directories or
objects. Third, it arbitrates requests by managing a queue of pending work and
evaluating requests to maximize dataspace throughput while minimizing the 
interferences with normal application I/O.

NORNS has currently been tested only under GNU/Linux.

## Building and installing from source

Distribution tarballs are available from the
[releases](https://github.com/bsc-ssrg/NORNS/releases) tab. If you are building
NORNS from a developer Git clone, you must first run the `bootstrap.sh` script,
which will invoke the GNU Autotools to bootstrap NORNS' configuration and build
mechanisms. If you are building NORNS from an official distribiution tarball,
there is no need to run the `bootstrap.sh` script, since all distribution
tarballs are already boostrapped.

### Dependencies

Compiling and running NORNS requires up-to-date versions of the following
software packages (note that, though it may compile and run, using excessively
old versions of these packages can cause indirect errors that are very
difficult to track down):

- A standard **C++11** conforming compiler (the code is routinely tested
  against GCC 4.9/Clang 3.3 and higher).
- Autotools (autoconf 2.69 or higher, automake 1.14.1 or higher, and libtool
  2.4.2 or higher) and CMake (3.10.0 or higher).
- The following Boost libraries (1.53 or higher): `system`, `filesystem`,
  `program_options`, and `thread`. Optionally, the `regex` library may also be
  required if self tests are enabled with the `--enable-tests` option.
- Google's Protocol Buffers for [C](https://github.com/protobuf-c/protobuf-c)
  (1.0.2 or higher) and for [C++](https://github.com/protocolbuffers/protobuf)
  (2.5.0 or higher).
- [LibYAML](https://github.com/yaml/libyaml) (0.1.4 or higher) and
  [yaml-cpp](https://github.com/jbeder/yaml-cpp) (0.5.1 or higher).
- [Mercury](https://github.com/mercury-hpc/mercury) 1.0 or higher 
  (**IMPORTANT** Mercury may require additional dependencies such as libfabric 
  depending on the desired transport protocol).
- [Hermes](https://github.com/bsc-ssrg/hermes) (our own C++ wrapper for
  Mercury. It should be automatically downloaded when cloning with the
  `--recursive` option).
- [libarchive](https://github.com/libarchive/libarchive) (3.1.2 or higher).

#### Installation in CentOS

```bash
# Installing dependencies available through package manager
$ yum install -y boost-devel protobuf protobuf-devel protobuf-c protobuf-c-devel \
                 yaml-cpp yaml-cpp-devel libyaml-devel libarchive

# Building and installing libfabric (required for Mercury's OFI/libfabric plugin)
$ git clone https://github.com/ofiwg/libfabric.git && cd libfabric
$ ./autogen.sh
$ mkdir build && cd build 
$ ../configure && make && make install

# Building and installing Mercury with OFI/libfabric plugin
$ git clone https://github.com/mercury-hpc/mercury.git && cd mercury
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DBUILD_TESTING:BOOL=OFF \
        -DMERCURY_USE_SM_ROUTING:BOOL=OFF -DMERCURY_USE_SELF_FORWARD:BOOL=OFF \
        -DMERCURY_USE_CHECKSUMS:BOOL=OFF -DMERCURY_USE_BOOST_PP:BOOL=ON \
        -DMERCURY_USE_EAGER_BULK:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=ON \
        -DNA_USE_OFI:BOOL=ON \
        ..
$ make && make install

# Building, testing and installing NORNS under '/usr/local/', with configuration
# files under '/etc/norns/' and temporary files under '/var/run/norns/'
$ git clone --recursive https://github.com/bsc-ssrg/NORNS.git && cd norns
$ ./bootstrap.sh
$ mkdir build && cd build 
$ ../configure \
    --enable-tests \
    --prefix=/usr/local \
    --sysconfdir=/etc/norns \
    --localstatedir=/var/run/norns
$ make && make check && make install

```

#### Installation in Ubuntu

```bash
# Installing dependencies available through package manager
$ apt-get install -y libboost-system-dev libboost-filesystem-dev \
                     libboost-program-options-dev libboost-thread-dev \
                     libboost-regex-dev libprotobuf-dev protobuf-compiler \
                     libprotobuf-c-dev protobuf-c-compiler \
                     libyaml-cpp-dev libyaml-dev libarchive-dev

# Building and installing libfabric (required for Mercury's OFI/libfabric plugin)
$ git clone https://github.com/ofiwg/libfabric.git && cd libfabric
$ ./autogen.sh
$ mkdir build && cd build 
$ ../configure && make && make install

# Building and installing Mercury with OFI/libfabric plugin
$ git clone https://github.com/mercury-hpc/mercury.git && cd mercury
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DBUILD_TESTING:BOOL=OFF \
        -DMERCURY_USE_SM_ROUTING:BOOL=OFF -DMERCURY_USE_SELF_FORWARD:BOOL=OFF \
        -DMERCURY_USE_CHECKSUMS:BOOL=OFF -DMERCURY_USE_BOOST_PP:BOOL=ON \
        -DMERCURY_USE_EAGER_BULK:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=ON \
        -DNA_USE_OFI:BOOL=ON \
        ..
$ make && make install

# Building, testing and installing NORNS under '/usr/local/', with configuration
# files under '/etc/norns/' and temporary files under '/var/run/norns/'
$ git clone --recursive https://github.com/bsc-ssrg/NORNS.git && cd norns
$ ./bootstrap.sh
$ mkdir build && cd build 
$ ../configure \
    --enable-tests \
    --prefix=/usr/local \
    --sysconfdir=/etc/norns \
    --localstatedir=/var/run/norns
$ make && make check && make install
```


#### Configuration

```bash
# Optional: providing file system permission override capabilities to NORNS
# control daemon
$ setcap cap_sys_ptrace,cap_chown=+ep /usr/local/bin/urd
```
