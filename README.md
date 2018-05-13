Norns Data Scheduler
====================

Build dependencies:

- A c++11-conforming compiler
- libboost-system >= 1.53
- libboost-filesystem >= 1.53
- libboost-program-options >= 1.53
- libboost-thread >= 1.53
- libprotobuf + protobuf compiler
- libprotobuf-c + protobuf-c compiler


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
