cmake_minimum_required(VERSION 2.6)
include("../misc/configs.txt")

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_library(test_application_tcp_echo MODULE
    application_echo_connection_processor.cpp
)

add_executable(all_tests
    server_core_config.yaml
    server_core_config_cerr.yaml
    server_core_config_io.yaml
    server_core_config_io20.yaml
    server_core_config_tcp.yaml
    server_core_confs/010_boost_loggers.yaml
    server_core_confs/020_boost_io_service.yaml
    server_core_confs/021_boost_io_service.yaml

    start_tests.cpp

    server_core.cpp
    io_service.cpp
    request_response.cpp
    tcp.cpp
)

target_link_libraries(all_tests ${GTEST_LIBRARIES} cppalls_core boost_system pthread)
add_test(NAME all_tests COMMAND all_tests WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
