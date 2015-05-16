cmake_minimum_required(VERSION 2.6)
include ("../../../misc/configs.txt")

add_library(cppalls_io_service MODULE
    boost_io_service.cpp
)


target_link_libraries(cppalls_io_service PRIVATE libboost_thread.so)
