#include "norns.h"
#include "nornsctl.h"
#include "test-env.hpp"
#include "catch.hpp"

SCENARIO("remove process from job", "[api::nornsctl_remove_process]") {
    GIVEN("a running urd instance") {

        test_env env;

        WHEN("a process is removed from a non-registered job") {

            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = nornsctl_remove_process(jobid, uid, gid, pid);

            THEN("NORNS_ENOSUCHJOB is returned") {
                REQUIRE(rv == NORNS_ENOSUCHJOB);
            }
        }

        WHEN("a non-existing process is removed from a registered job") {

            const char* test_hosts[] = { "host00", "host01" };
            const size_t test_nhosts = sizeof(test_hosts) / sizeof(test_hosts[0]);

            nornsctl_job_limit_t l0 = NORNSCTL_JOB_LIMIT("b0://", 1024);
            nornsctl_job_limit_t l1 = NORNSCTL_JOB_LIMIT("b1://", 2048);
            nornsctl_job_limit_t l2 = NORNSCTL_JOB_LIMIT("b2://", 1024);

            nornsctl_job_limit_t* test_lims[] = { &l0, &l1, &l2 };
            const size_t test_nlims = sizeof(test_lims) / sizeof(test_lims[0]);

            nornsctl_job_t job = NORNSCTL_JOB(test_hosts, test_nhosts, test_lims, test_nlims);
            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = nornsctl_register_job(jobid, &job);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_remove_process(jobid, uid, gid, pid);

            THEN("NORNS_ENOSUCHPROCESS is returned") {
                REQUIRE(rv == NORNS_ENOSUCHPROCESS);
            }
        }

        WHEN("an existing process is removed from a registered job") {

            /* valid information for nornsctl_register_job */
            const char* test_hosts[] = { "host00", "host01" };
            const size_t test_nhosts = sizeof(test_hosts) / sizeof(test_hosts[0]);

            nornsctl_job_limit_t l0 = NORNSCTL_JOB_LIMIT("b0://", 1024);
            nornsctl_job_limit_t l1 = NORNSCTL_JOB_LIMIT("b1://", 2048);
            nornsctl_job_limit_t l2 = NORNSCTL_JOB_LIMIT("b2://", 1024);

            nornsctl_job_limit_t* test_lims[] = { &l0, &l1, &l2 };
            const size_t test_nlims = sizeof(test_lims) / sizeof(test_lims[0]);

            nornsctl_job_t job1 = NORNSCTL_JOB(test_hosts, test_nhosts, test_lims, test_nlims);
            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = nornsctl_register_job(jobid, &job1);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_add_process(jobid, uid, gid, pid);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_remove_process(jobid, uid, gid, pid);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
            }
        }

        WHEN("an existing process is removed twice from a registered job") {

            /* valid information for nornsctl_register_job */
            const char* test_hosts[] = { "host00", "host01" };
            const size_t test_nhosts = sizeof(test_hosts) / sizeof(test_hosts[0]);

            nornsctl_job_limit_t l0 = NORNSCTL_JOB_LIMIT("b0://", 1024);
            nornsctl_job_limit_t l1 = NORNSCTL_JOB_LIMIT("b1://", 2048);
            nornsctl_job_limit_t l2 = NORNSCTL_JOB_LIMIT("b2://", 1024);

            nornsctl_job_limit_t* test_lims[] = { &l0, &l1, &l2 };
            const size_t test_nlims = sizeof(test_lims) / sizeof(test_lims[0]);

            nornsctl_job_t job1 = NORNSCTL_JOB(test_hosts, test_nhosts, test_lims, test_nlims);
            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = nornsctl_register_job(jobid, &job1);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_add_process(jobid, uid, gid, pid);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_remove_process(jobid, uid, gid, pid);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_remove_process(jobid, uid, gid, pid);

            THEN("NORNS_ENOSUCHPROCESS is returned") {
                REQUIRE(rv == NORNS_ENOSUCHPROCESS);
            }
        }

        env.notify_success();
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("attempting to remove a process") {

            const uint32_t jobid = 42;
            const uid_t uid = 1001;
            const gid_t gid = 2001;
            const pid_t pid = 25401;

            int rv = nornsctl_remove_process(jobid, uid, gid, pid);

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
#endif
}
