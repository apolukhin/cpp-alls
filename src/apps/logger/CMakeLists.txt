include ("../../../misc/configs.txt")

add_library(cppalls_logger MODULE
    logger.cpp
)

target_link_libraries(cppalls_logger PRIVATE libboost_log.so)

