#include "norns.h"
#include "nornsctl.h"
#include "test-env.hpp"
#include "catch.hpp"

SCENARIO("register job", "[api::nornsctl_register_job]") {
    GIVEN("a running urd instance") {

        test_env env;

        WHEN("a job is registered with invalid information") {

            nornsctl_job_t job = NORNSCTL_JOB(NULL, 0, NULL, 0);

            const uint32_t jobid = 42;

            int rv = nornsctl_register_job(jobid, &job);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("a job is registered with valid information") {

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

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        env.notify_success();
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("attempting to register a job") {

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

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
#endif
}
