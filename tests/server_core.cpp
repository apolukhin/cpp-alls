#include "gtest/gtest.h"

#include "cppalls/server.hpp"
#include "cppalls/logging.hpp"
#include "cppalls/api/logger.hpp"

using namespace cppalls;

TEST(server, construction) {
    int argc = 2;
    const char* argv[] = {"test_all", "--config=../../cpp-alls/tests/config.yaml"};
    $.init(argc, argv);
    auto apps = $.list_apps();
    ASSERT_TRUE(!apps.empty());

     auto apps2 = $.list_apps();
     ASSERT_TRUE(apps == apps2);
}

TEST(server, app_instance) {
    std::shared_ptr<api::logger> l1;
    $["main_logger"] >> l1;
    auto l2 = $["main_logger"].as<api::logger>();

    ASSERT_TRUE(l1->version() == l2->version());
    LINFO(*l1) << "Works!";
    LWARN(*l2) << "Works!!!!!!!!!!";
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
