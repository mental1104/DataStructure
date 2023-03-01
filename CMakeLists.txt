cmake_minimum_required(VERSION 3.2.0)
project(DSA CXX)

set(CMAKE_C_COMPILER "which gcc")
set(CMAKE_CXX_COMPILE_FEATURES "which g++")
set(CMAKE_CXX_STANDARD 11)
set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/)

add_subdirectory(src/common/vector)

add_executable(maj maj.cpp)

include_directories(src/common/lib/util)
include_directories(src/common/dp)
target_include_directories(maj PUBLIC src/common/vector)
#target_link_libraries(maj PRIVATE vector)
