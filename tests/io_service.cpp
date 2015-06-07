#include "gtest/gtest.h"

#include "cppalls/core/server.hpp"
#include "cppalls/core/logging.hpp"
#include "cppalls/core/exceptions.hpp"
#include "cppalls/api/io_service.hpp"

using namespace cppalls;

struct server_guard {
    server_guard() {
        server::start("../../cpp-alls/tests/server_core_config.yaml");
    }

    ~server_guard() {
        server::stop();
    }
};


TEST(io_service, io_service_multiple_reloads) {
    const unsigned int tests_rerun_count = 10;
    for (unsigned int i = 0; i < tests_rerun_count; ++i) {
        server_guard guard;
        auto io1_1 = app::get<api::io_service_provider>("io");
        ASSERT_TRUE(!!io1_1);

        server::reload("../../cpp-alls/tests/server_core_config_io20.yaml");
        auto io20 = app::get<api::io_service_provider>("io");
        ASSERT_TRUE(!!io20);
        ASSERT_TRUE(io1_1 == io20);

        server::reload("../../cpp-alls/tests/server_core_config_io.yaml");
        auto io1_2 = app::get<api::io_service_provider>("io");
        ASSERT_TRUE(!!io20);
        ASSERT_TRUE(io1_1 == io20);
        ASSERT_TRUE(io1_2 == io20);
    }
}
