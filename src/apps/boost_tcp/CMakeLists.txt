cmake_minimum_required(VERSION 2.6)
include ("../../../misc/configs.txt")

add_library(cppalls_tcp MODULE
    async_connection_fast.hpp
    handler_allocator.hpp

    tcp.cpp

    to_std_error_code.hpp
    to_std_error_code.cpp

    tcp_ssl.cpp
    udp.cpp
)


target_link_libraries(cppalls_tcp PRIVATE ssl)
