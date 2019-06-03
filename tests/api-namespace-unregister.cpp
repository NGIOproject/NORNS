#include "norns.h"
#include "nornsctl.h"
#include "test-env.hpp"
#include "catch.hpp"

SCENARIO("unregister namespace", "[api::nornsctl_unregister_namespace]") {
    GIVEN("a running urd instance") {

        test_env env;
        const bfs::path path_b0 = env.create_directory("mnt/b0", env.basedir());

        WHEN("a namespace is unregistered with an invalid prefix") {

            int rv = nornsctl_unregister_namespace(NULL);

            THEN("NORNS_EBADARGS is returned") {
                REQUIRE(rv == NORNS_EBADARGS);
            }
        }

        WHEN("attempting to unregister a non-existing namespace") {

            int rv = nornsctl_unregister_namespace("b0://");

            THEN("NORNS_ENOSUCHNAMESPACE is returned") {
                REQUIRE(rv == NORNS_ENOSUCHNAMESPACE);
            }
        }

        WHEN("unregistering a registered namespace") {

            const char* nsid = "b0://";

            nornsctl_backend_t b0 = 
                NORNSCTL_BACKEND(NORNS_BACKEND_NVML, false, path_b0.c_str(), 4096);

            int rv = nornsctl_register_namespace(nsid, &b0);

            REQUIRE(rv == NORNS_SUCCESS);

            rv = nornsctl_unregister_namespace(nsid);

            THEN("NORNS_SUCCESS is returned") {
                REQUIRE(rv == NORNS_SUCCESS);
            }

        }

        env.notify_success();
    }

#ifndef USE_REAL_DAEMON
    GIVEN("a non-running urd instance") {
        WHEN("attempting to unregister a namespace") {

            int rv = nornsctl_unregister_namespace("b0://");

            THEN("NORNS_ECONNFAILED is returned") {
                REQUIRE(rv == NORNS_ECONNFAILED);
            }
        }
    }
#endif
}
