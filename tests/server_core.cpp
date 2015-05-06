#include "gtest/gtest.h"

#include "cppalls/server.hpp"
#include "cppalls/logging.hpp"
#include "cppalls/api/logger.hpp"

using namespace cppalls;

TEST(server, construction) {
    int argc = 2;
    const char* argv[] = {"test_all", "--config=../../cpp-alls/tests/config.yaml"};
    server::init(argc, argv);
    auto apps = server::list_apps();
    ASSERT_TRUE(!apps.empty());

     auto apps2 = server::list_apps();
     ASSERT_TRUE(apps == apps2);
}

TEST(server, app_instance) {
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
