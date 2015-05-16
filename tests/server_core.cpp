#include "gtest/gtest.h"

#include "cppalls/core/server.hpp"
#include "cppalls/core/logging.hpp"
#include "cppalls/core/exceptions.hpp"
#include "cppalls/api/logger.hpp"

using namespace cppalls;

struct server_guard {
    server_guard() {
        const int argc = 2;
        const char* const argv[2] = {"test_all", "--config=../../cpp-alls/tests/server_core_config.yaml"};
        server::start(argc, argv);
    }

    ~server_guard() {
        server::stop();
    }
};

struct fake_api : api::application {
    void start(const YAML::Node& /*config*/) override {}
    void stop() override {}
};


TEST(server, basic_start_stop_reload) {
    server_guard guard;

    auto apps = server::available_apps();
    ASSERT_TRUE(!apps.empty());
    ASSERT_TRUE(apps == server::available_apps());

    auto logger_app = server::get<api::logger>("warning_logger");
    ASSERT_TRUE(!!logger_app);
    EXPECT_THROW(!!server::get<fake_api>("warning_logger"), error_runtime);
    EXPECT_THROW(!!server::get<api::logger>("app_that_does_not_exist"), error_runtime);


    server::stop();
    ASSERT_TRUE(server::available_apps().empty());

    EXPECT_THROW(!!server::get<api::logger>("warning_logger"), error_runtime);
    EXPECT_THROW(!!server::get<fake_api>("warning_logger"), error_runtime);
    EXPECT_THROW(!!server::get<api::logger>("app_that_does_not_exist"), error_runtime);

    server::stop();
    ASSERT_TRUE(server::available_apps().empty());

    EXPECT_THROW(!!server::get<api::logger>("warning_logger"), error_runtime);
    EXPECT_THROW(!!server::get<fake_api>("warning_logger"), error_runtime);
    EXPECT_THROW(!!server::get<api::logger>("app_that_does_not_exist"), error_runtime);


    server::reload();
    // servr was stopped, so old app != new app
    ASSERT_TRUE(logger_app != server::get<api::logger>("warning_logger"));
    ASSERT_TRUE(apps == server::available_apps());
    ASSERT_TRUE(!!server::get<api::logger>("warning_logger"));
    EXPECT_THROW(!!server::get<fake_api>("warning_logger"), error_runtime);
    EXPECT_THROW(!!server::get<api::logger>("app_that_does_not_exist"), error_runtime);


    const char* const argv[2] = {"test_all", "--config=../../cpp-alls/tests/server_core_config_cerr.yaml"};
    server::reload(2, argv);
    // server was stopped, so old app != new app
    ASSERT_TRUE(logger_app != server::get<api::logger>("warning_logger"));
    ASSERT_TRUE(apps == server::available_apps());
    ASSERT_TRUE(!!server::get<api::logger>("warning_logger"));
    EXPECT_THROW(!!server::get<fake_api>("warning_logger"), error_runtime);
    EXPECT_THROW(!!server::get<api::logger>("app_that_does_not_exist"), error_runtime);
}

TEST(server, reload_same_apps) {
    server_guard guard;
    auto logger_app = server::get<api::logger>("warning_logger");

    server::reload();
    // Server was not stopped, so new app == old app
    ASSERT_TRUE(logger_app == server::get<api::logger>("warning_logger"));
    EXPECT_THROW(!!server::get<api::logger>("__basic-logger"), error_runtime);


    const char* const argv[2] = {"test_all", "--config=../../cpp-alls/tests/server_core_config_cerr.yaml"};
    server::reload(2, argv);
    // Server was not stopped, so new app == old app
    ASSERT_TRUE(logger_app == server::get<api::logger>("warning_logger"));
    ASSERT_TRUE(server::get<api::logger>("__basic-logger") != server::get<api::logger>("warning_logger"));
}


TEST(server, app_instance) {
    server_guard guard;

    std::shared_ptr<api::logger> l1 = server::get<api::logger>("warning_logger");
    auto l2 = server::get("warning_logger");
    ASSERT_TRUE(l1->version() == l2->version());
    ASSERT_TRUE(l1 == l2);
    LINFO(*l1) << "Works!";

    std::shared_ptr<api::logger> l1_deb = server::get<api::logger>("debug_logger");
    auto l2_deb = server::get("debug_logger");
    ASSERT_TRUE(l1_deb->version() == l2_deb->version());
    ASSERT_TRUE(l1_deb == l2_deb);
    ASSERT_TRUE(l1 != l1_deb);
    ASSERT_TRUE(l2 != l2_deb);
    LINFO(*l1_deb) << "Works!";
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
