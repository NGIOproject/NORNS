#include "norns.h"
#include "nornsctl.h"
#include "test-env.hpp"
#include "catch.hpp"

SCENARIO("unregister job", "[api::nornsctl_unregister_job]") {
    GIVEN("a running urd instance") {

        test_env env;

        WHEN("a non-registered job is unregistered") {

            const uint32_t jobid = 42;

            int rv = nornsctl_unregister_job(jobid);

            THEN("NORNS_ENOSUCHJOB is returned") {
                REQUIRE(rv == NORNS_ENOSUCHJOB);
            }
        }

        WHEN("a registered job is unregistered") {

            /* valid information for nornsctl_register_job */
            const char* test_hosts[] = { "host00", "host01" };
            const size_t test_nhosts = sizeof(test_hosts) / sizeof(test_hosts[0]);

            nornsctl_job_limit_t l0 = NORNSCTL_JOB_LIMIT("b0://", 1024);
            nornsctl_job_limit_t l1 = NORNSCTL_JOB_LIMIT("b1://", 2048);
            nornsctl_job_limit_t l2 = NORNSCTL_JOB_LIMIT("b2://", 1024);

            nornsctl_job_limit_t* test_lims[] = { &l0, &l1, &l2 };
            const size_t test_nlims = sizeof(test_lims) / sizeof(test_lims[0]);

            nornsctl_job_t job = NORNSCTL_JOB(test_hosts, test_nhosts, test_lims, test_nlims);
            const uint32_t jobid = 42;

            int rv = nornsctl_register_job(jobid, &job);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_job(jobid);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        WHEN("a registered job is updated and unregistered") {

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

            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_job(jobid);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        env.notify_success();
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("attempting to unregister a job") {

            const uint32_t jobid = 42;

            int rv = nornsctl_unregister_job(jobid);

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
#endif
}
