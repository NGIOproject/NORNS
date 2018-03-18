/*************************************************************************
 * Copyright (C) 2017-2018 Barcelona Supercomputing Center               *
 *                         Centro Nacional de Supercomputacion           *
 * All rights reserved.                                                  *
 *                                                                       *
 * This file is part of the NORNS Data Scheduler, a service that allows  *
 * other programs to start, track and manage asynchronous transfers of   *
 * data resources transfers requests between different storage backends. *
 *                                                                       *
 * See AUTHORS file in the top level directory for information           *
 * regarding developers and contributors.                                *
 *                                                                       *
 * The NORNS Data Scheduler is free software: you can redistribute it    *
 * and/or modify it under the terms of the GNU Lesser General Public     *
 * License as published by the Free Software Foundation, either          *
 * version 3 of the License, or (at your option) any later version.      *
 *                                                                       *
 * The NORNS Data Scheduler is distributed in the hope that it will be   *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General             *
 * Public License along with the NORNS Data Scheduler.  If not, see      *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

#include "norns.h"
#include "nornsctl.h"
#include "catch.hpp"
#include "fake-daemon.hpp"

// enable to test connections with an already running daemon
//#define USE_REAL_DAEMON

SCENARIO("add process to job", "[api::norns_add_process]") {
    GIVEN("a running urd instance") {

#ifndef USE_REAL_DAEMON
        fake_daemon td;
        td.run();
#endif

        WHEN("a process is added to a non-registered job") {

            struct norns_cred cred;
            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = norns_add_process(&cred, jobid, uid, gid, pid);

            THEN("NORNS_ENOSUCHJOB is returned") {
                REQUIRE(rv == NORNS_ENOSUCHJOB);
            }
        }

        WHEN("a process is added to a registered job") {

            /* valid information for norns_register_job */
            const char* test_hosts[] = { "host00", "host01" };
            const size_t test_nhosts = sizeof(test_hosts) / sizeof(test_hosts[0]);

            norns_backend_t b0 = NORNS_BACKEND("b0://", NORNS_BACKEND_NVML, "/mnt/b0", 1024);
            norns_backend_t b1 = NORNS_BACKEND("b1://", NORNS_BACKEND_NVML, "/mnt/b1", 2048);
            norns_backend_t b2 = NORNS_BACKEND("b2://", NORNS_BACKEND_NVML, "/mnt/b2", 1024);

            norns_backend_t* test_backends[] = { &b0, &b1, &b2 };
            const size_t test_nbackends = sizeof(test_backends) / sizeof(test_backends[0]);

            struct norns_cred cred;
            norns_job_t job = NORNS_JOB(test_hosts, test_nhosts, test_backends, test_nbackends);
            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = norns_register_job(&cred, jobid, &job);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_add_process(&cred, jobid, uid, gid, pid);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        WHEN("a process is added to a registered job that has been updated with valid information") {

            /* valid information for norns_register_job */
            const char* test_hosts1[] = { "host00", "host01" };
            const char* test_hosts2[] = { "host02", "host03", "host04" };
            const size_t test_nhosts1 = sizeof(test_hosts1) / sizeof(test_hosts1[0]);
            const size_t test_nhosts2 = sizeof(test_hosts2) / sizeof(test_hosts2[0]);

            norns_backend_t b0 = NORNS_BACKEND("b0://", NORNS_BACKEND_NVML, "/mnt/b0", 1024);
            norns_backend_t b1 = NORNS_BACKEND("b1://", NORNS_BACKEND_NVML, "/mnt/b1", 2048);
            norns_backend_t b2 = NORNS_BACKEND("b2://", NORNS_BACKEND_NVML, "/mnt/b2", 1024);
            norns_backend_t b3 = NORNS_BACKEND("b3://", NORNS_BACKEND_NVML, "/mnt/b3", 3072);

            norns_backend_t* test_backends1[] = { &b0, &b1, &b2 };
            const size_t test_nbackends1 = sizeof(test_backends1) / sizeof(test_backends1[0]);
            norns_backend_t* test_backends2[] = { &b1, &b2, &b3 };
            const size_t test_nbackends2 = sizeof(test_backends2) / sizeof(test_backends2[0]);

            norns_job_t job1 = NORNS_JOB(test_hosts1, test_nhosts1, test_backends1, test_nbackends1);
            norns_job_t job2 = NORNS_JOB(test_hosts2, test_nhosts2, test_backends2, test_nbackends2);
            struct norns_cred cred;
            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = norns_register_job(&cred, jobid, &job1);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_update_job(&cred, jobid, &job2);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_add_process(&cred, jobid, uid, gid, pid);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        WHEN("a process is added to a registered job that has been unregistered") {

            /* valid information for norns_register_job */
            const char* test_hosts[] = { "host00", "host01" };
            const size_t test_nhosts = sizeof(test_hosts) / sizeof(test_hosts[0]);

            norns_backend_t b0 = NORNS_BACKEND("b0://", NORNS_BACKEND_NVML, "/mnt/b0", 1024);
            norns_backend_t b1 = NORNS_BACKEND("b1://", NORNS_BACKEND_NVML, "/mnt/b1", 2048);
            norns_backend_t b2 = NORNS_BACKEND("b2://", NORNS_BACKEND_NVML, "/mnt/b2", 1024);

            norns_backend_t* test_backends[] = { &b0, &b1, &b2 };
            const size_t test_nbackends = sizeof(test_backends) / sizeof(test_backends[0]);

            struct norns_cred cred;
            norns_job_t job = NORNS_JOB(test_hosts, test_nhosts, test_backends, test_nbackends);
            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = norns_register_job(&cred, jobid, &job);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_unregister_job(&cred, jobid);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_add_process(&cred, jobid, uid, gid, pid);

            THEN("NORNS_ENOSUCHJOB is returned") {
                REQUIRE(rv == NORNS_ENOSUCHJOB);
            }
        }

#ifndef USE_REAL_DAEMON
        int ret = td.stop();
        REQUIRE(ret == 0);
#endif
    }

    GIVEN("a non-running urd instance") {
        WHEN("attempting to add a process") {

            struct norns_cred cred;
            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = norns_add_process(&cred, jobid, uid, gid, pid);

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
}
