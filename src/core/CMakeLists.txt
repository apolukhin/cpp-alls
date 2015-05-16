cmake_minimum_required(VERSION 2.6)
include("../../misc/configs.txt")

add_library(cppalls_core SHARED

    ../../include/cppalls/core/server.hpp
    ../../include/cppalls/core/logging.hpp
    ../../include/cppalls/core/exceptions.hpp
    ../../include/cppalls/api/application.hpp
    ../../include/cppalls/api/logger.hpp
    ../../include/cppalls/api/io_service.hpp

    server.cpp
    cpp_logger.cpp
)

target_link_libraries(cppalls_core boost_filesystem boost_program_options yaml-cpp dl)
