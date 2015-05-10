cmake_minimum_required(VERSION 2.6)
include ("../../../misc/configs.txt")

add_library(cppalls_logger MODULE
    logger.cpp
)

target_link_libraries(cppalls_logger PRIVATE libboost_log.so libboost_log_setup.so)

