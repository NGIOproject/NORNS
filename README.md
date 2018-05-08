Norns Data Scheduler
====================

Dependencies:

- Intel TBB
yum install tbb-devel

- Installation in CentOS 7
cd <build-dir>
./configure --prefix=<install-dir> --sysconfdir=<config-dir>
make
make install
cp <build-dir>/etc/norns.service /usr/lib/systemd/system/norns.service

sudo setcap cap_sys_ptrace,cap_chown=+ep ./urd
