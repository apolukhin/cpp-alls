cmake_minimum_required(VERSION 2.6)
include("../misc/configs.txt")

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(all_tests
    server_core_config.yaml
    server_core_config_cerr.yaml
    server_core_config_io.yaml
    server_core_config_io20.yaml
    server_core_confs/010_boost_loggers.yaml
    server_core_confs/020_boost_io_service.yaml
    server_core_confs/021_boost_io_service.yaml

    server_core.cpp
    io_service.cpp
)

target_link_libraries(all_tests ${GTEST_LIBRARIES} cppalls_core boost_system pthread)
add_test(NAME all_tests COMMAND all_tests WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
