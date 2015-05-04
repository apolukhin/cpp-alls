cmake_minimum_required(VERSION 2.6)
include("../misc/configs.txt")

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(all_tests config.yaml server_core.cpp)
target_link_libraries(all_tests ${GTEST_LIBRARIES} cppalls_core boost_system pthread)
