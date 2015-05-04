cmake_minimum_required(VERSION 2.6)
include ("./misc/configs.txt")

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
add_definitions(-DBOOST_ALL_DYN_LINK)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fvisibility=hidden")
#set_property(GLOBAL PROPERTY CXX_STANDARD 11)
#set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)

add_subdirectory("src")
add_subdirectory("tests")


