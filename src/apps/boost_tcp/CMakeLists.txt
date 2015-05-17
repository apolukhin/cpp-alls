cmake_minimum_required(VERSION 2.6)
include ("../../../misc/configs.txt")

add_library(cppalls_tcp MODULE
    tcp.cpp
)


target_link_libraries(cppalls_tcp PRIVATE)
