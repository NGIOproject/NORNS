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
#include "test-env.hpp"
#include "catch.hpp"

SCENARIO("update job", "[api::nornsctl_update_job]") {
    GIVEN("a running urd instance") {

        test_env env;

        WHEN("a non-registered job is updated with invalid information") {

            nornsctl_job_t job = NORNSCTL_JOB(NULL, 0, NULL, 0);
            const uint32_t jobid = 42;

            int rv = nornsctl_update_job(jobid, &job);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a non-registered job is updated with valid information") {

            const char* test_hosts[] = { "host00", "host01" };
            const size_t test_nhosts = sizeof(test_hosts) / sizeof(test_hosts[0]);

            nornsctl_job_limit_t l0 = NORNSCTL_JOB_LIMIT("b0://", 1024);
            nornsctl_job_limit_t l1 = NORNSCTL_JOB_LIMIT("b1://", 2048);
            nornsctl_job_limit_t l2 = NORNSCTL_JOB_LIMIT("b2://", 1024);

            nornsctl_job_limit_t* test_lims[] = { &l0, &l1, &l2 };
            const size_t test_nlims = sizeof(test_lims) / sizeof(test_lims[0]);

            nornsctl_job_t job = NORNSCTL_JOB(test_hosts, test_nhosts, test_lims, test_nlims);
            const uint32_t jobid = 42;

            int rv = nornsctl_update_job(jobid, &job);

            THEN("NORNS_ENOSUCHJOB is returned") {
                REQUIRE(rv == NORNS_ENOSUCHJOB);
            }
        }

        WHEN("a registered job is updated with invalid information") {

            /* valid information for nornsctl_register_job */
            const char* test_hosts[] = { "host00", "host01" };
            const size_t test_nhosts = sizeof(test_hosts) / sizeof(test_hosts[0]);

            nornsctl_job_limit_t l0 = NORNSCTL_JOB_LIMIT("b0://", 1024);
            nornsctl_job_limit_t l1 = NORNSCTL_JOB_LIMIT("b1://", 2048);
            nornsctl_job_limit_t l2 = NORNSCTL_JOB_LIMIT("b2://", 1024);

            nornsctl_job_limit_t* test_lims[] = { &l0, &l1, &l2 };
            const size_t test_nlims = sizeof(test_lims) / sizeof(test_lims[0]);

            nornsctl_job_t job1 = NORNSCTL_JOB(test_hosts, test_nhosts, test_lims, test_nlims);
            nornsctl_job_t job2 = NORNSCTL_JOB(NULL, 0, NULL, 0);
            const uint32_t jobid = 42;

            int rv = nornsctl_register_job(jobid, &job1);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_update_job(jobid, &job2);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a registered job is updated with valid information") {

            /* valid information for nornsctl_register_job */
            const char* test_hosts1[] = { "host00", "host01" };
            const char* test_hosts2[] = { "host02", "host03", "host04" };
            const size_t test_nhosts1 = sizeof(test_hosts1) / sizeof(test_hosts1[0]);
            const size_t test_nhosts2 = sizeof(test_hosts2) / sizeof(test_hosts2[0]);

            nornsctl_job_limit_t l0 = NORNSCTL_JOB_LIMIT("b0://", 1024);
            nornsctl_job_limit_t l1 = NORNSCTL_JOB_LIMIT("b1://", 2048);
            nornsctl_job_limit_t l2 = NORNSCTL_JOB_LIMIT("b2://", 1024);
            nornsctl_job_limit_t l3 = NORNSCTL_JOB_LIMIT("b3://", 3072);

            nornsctl_job_limit_t* test_backends1[] = { &l0, &l1, &l2 };
            const size_t test_nbackends1 = sizeof(test_backends1) / sizeof(test_backends1[0]);
            nornsctl_job_limit_t* test_backends2[] = { &l1, &l2, &l3 };
            const size_t test_nbackends2 = sizeof(test_backends2) / sizeof(test_backends2[0]);

            nornsctl_job_t job1 = NORNSCTL_JOB(test_hosts1, test_nhosts1, test_backends1, test_nbackends1);
            nornsctl_job_t job2 = NORNSCTL_JOB(test_hosts2, test_nhosts2, test_backends2, test_nbackends2);
            const uint32_t jobid = 42;

            int rv = nornsctl_register_job(jobid, &job1);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_update_job(jobid, &job2);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        env.notify_success();
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("attempting to update a job") {

            const char* test_hosts[] = { "host00", "host01" };
            const size_t test_nhosts = sizeof(test_hosts) / sizeof(test_hosts[0]);

            nornsctl_job_limit_t l0 = NORNSCTL_JOB_LIMIT("b0://", 1024);
            nornsctl_job_limit_t l1 = NORNSCTL_JOB_LIMIT("b1://", 2048);
            nornsctl_job_limit_t l2 = NORNSCTL_JOB_LIMIT("b2://", 1024);

            nornsctl_job_limit_t* test_lims[] = { &l0, &l1, &l2 };

            const size_t test_nlims = sizeof(test_lims) / sizeof(test_lims[0]);

            nornsctl_job_t job = NORNSCTL_JOB(test_hosts, test_nhosts, test_lims, test_nlims);

            const uint32_t jobid = 42;

            int rv = nornsctl_update_job(jobid, &job);

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
#endif
}
