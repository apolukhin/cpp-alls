include("../../misc/configs.txt")

add_library(cppalls_core

    ../../include/cppalls/server.hpp
    ../../include/cppalls/logging.hpp
    ../../include/cppalls/api/application.hpp
    ../../include/cppalls/api/logger.hpp

    server.cpp
)

target_link_libraries(cppalls_core boost_filesystem boost_program_options yaml-cpp dl)
