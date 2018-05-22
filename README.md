Norns Data Scheduler
====================

Build dependencies:

- A c++11-conforming compiler
- libboost-system >= 1.53
- libboost-filesystem >= 1.53
- libboost-program-options >= 1.53
- libboost-thread >= 1.53
- libprotobuf + protobuf compiler >= 2.5.0
- libprotobuf-c + protobuf-c compiler >= 1.0.2
- libyaml-cpp >= 0.5.1


- Installation in CentOS 7
git clone git@git.ph.ed.ac.uk:nextgenio/norns.git && cd norns
./bootstrap
mkdir <build-dir>
cd <build-dir>
./configure --prefix=<install-dir> --sysconfdir=<config-dir>
make
make install
cp <build-dir>/etc/norns.service /usr/lib/systemd/system/norns.service

sudo setcap cap_sys_ptrace,cap_chown=+ep ./urd
