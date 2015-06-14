cmake_minimum_required(VERSION 2.6)
include("../../misc/configs.txt")

add_library(cppalls_core SHARED

    ../../include/cppalls/core/export.hpp
    ../../include/cppalls/core/server.hpp
    ../../include/cppalls/core/logging.hpp
    ../../include/cppalls/core/exceptions.hpp
    ../../include/cppalls/core/request.hpp
    ../../include/cppalls/core/stack_request.hpp
    ../../include/cppalls/core/response.hpp
    ../../include/cppalls/core/stack_response.hpp
    ../../include/cppalls/core/connection.hpp
    ../../include/cppalls/core/tcp_connection.hpp
    ../../include/cppalls/core/tcp_ssl_connection.hpp
    ../../include/cppalls/core/udp_connection.hpp
    ../../include/cppalls/core/detail/boost_small_vector_fwd.hpp
    ../../include/cppalls/core/detail/boost_asio_fwd.hpp
    ../../include/cppalls/core/detail/stack_pimpl.hpp
    ../../include/cppalls/core/detail/shared_allocator_friend.hpp
    ../../include/cppalls/core/detail/sync_connection.hpp
    ../../include/cppalls/api/application.hpp
    ../../include/cppalls/api/logger.hpp
    ../../include/cppalls/api/io_service.hpp
    ../../include/cppalls/api/processor.hpp
    ../../include/cppalls/api/connection_processor.hpp

    server.cpp
    cpp_logger.cpp
    cpp_logger.hpp
    request.cpp
    stack_request.cpp
    response.cpp
    stack_response.cpp
    sync_connection.ipp
    tcp_connection.cpp
    tcp_ssl_connection.cpp
    udp_connection.cpp
)

target_link_libraries(cppalls_core boost_filesystem boost_program_options yaml-cpp dl ssl)
