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

SCENARIO("remove process from job", "[api::norns_remove_process]") {
    GIVEN("a running urd instance") {

#ifndef USE_REAL_DAEMON
        fake_daemon td;
        td.run();
#endif

        WHEN("a process is removed from a non-registered job") {

            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = norns_remove_process(jobid, uid, gid, pid);

            THEN("NORNS_ENOSUCHJOB is returned") {
                REQUIRE(rv == NORNS_ENOSUCHJOB);
            }
        }

        WHEN("a non-existing process is removed from a registered job") {

            const char* test_hosts[] = { "host00", "host01" };
            const size_t test_nhosts = sizeof(test_hosts) / sizeof(test_hosts[0]);

            norns_job_limit_t l0 = NORNS_JOB_LIMIT("b0://", 1024);
            norns_job_limit_t l1 = NORNS_JOB_LIMIT("b1://", 2048);
            norns_job_limit_t l2 = NORNS_JOB_LIMIT("b2://", 1024);

            norns_job_limit_t* test_lims[] = { &l0, &l1, &l2 };
            const size_t test_nlims = sizeof(test_lims) / sizeof(test_lims[0]);

            norns_job_t job = NORNS_JOB(test_hosts, test_nhosts, test_lims, test_nlims);
            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = norns_register_job(jobid, &job);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_remove_process(jobid, uid, gid, pid);

            THEN("NORNS_ENOSUCHPROCESS is returned") {
                REQUIRE(rv == NORNS_ENOSUCHPROCESS);
            }
        }

        WHEN("an existing process is removed from a registered job") {

            /* valid information for norns_register_job */
            const char* test_hosts[] = { "host00", "host01" };
            const size_t test_nhosts = sizeof(test_hosts) / sizeof(test_hosts[0]);

            norns_job_limit_t l0 = NORNS_JOB_LIMIT("b0://", 1024);
            norns_job_limit_t l1 = NORNS_JOB_LIMIT("b1://", 2048);
            norns_job_limit_t l2 = NORNS_JOB_LIMIT("b2://", 1024);

            norns_job_limit_t* test_lims[] = { &l0, &l1, &l2 };
            const size_t test_nlims = sizeof(test_lims) / sizeof(test_lims[0]);

            norns_job_t job1 = NORNS_JOB(test_hosts, test_nhosts, test_lims, test_nlims);
            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = norns_register_job(jobid, &job1);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_add_process(jobid, uid, gid, pid);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_remove_process(jobid, uid, gid, pid);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        WHEN("an existing process is removed twice from a registered job") {

            /* valid information for norns_register_job */
            const char* test_hosts[] = { "host00", "host01" };
            const size_t test_nhosts = sizeof(test_hosts) / sizeof(test_hosts[0]);

            norns_job_limit_t l0 = NORNS_JOB_LIMIT("b0://", 1024);
            norns_job_limit_t l1 = NORNS_JOB_LIMIT("b1://", 2048);
            norns_job_limit_t l2 = NORNS_JOB_LIMIT("b2://", 1024);

            norns_job_limit_t* test_lims[] = { &l0, &l1, &l2 };
            const size_t test_nlims = sizeof(test_lims) / sizeof(test_lims[0]);

            norns_job_t job1 = NORNS_JOB(test_hosts, test_nhosts, test_lims, test_nlims);
            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = norns_register_job(jobid, &job1);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_add_process(jobid, uid, gid, pid);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_remove_process(jobid, uid, gid, pid);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = norns_remove_process(jobid, uid, gid, pid);

            THEN("NORNS_ENOSUCHPROCESS is returned") {
                REQUIRE(rv == NORNS_ENOSUCHPROCESS);
            }
        }

#ifndef USE_REAL_DAEMON
        int ret = td.stop();
        REQUIRE(ret == 0);
#endif
    }

    GIVEN("a non-running urd instance") {
        WHEN("attempting to remove a process") {

            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = norns_remove_process(jobid, uid, gid, pid);

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
}
