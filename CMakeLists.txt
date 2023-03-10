cmake_minimum_required(VERSION 3.2.0)
project(DSA CXX)

set(CMAKE_C_COMPILER "which gcc")
set(CMAKE_CXX_COMPILE_FEATURES "which g++")
set(CMAKE_CXX_STANDARD 11)
set(TOP_DIR ${CMAKE_CURRENT_LIST_DIR}/)

enable_testing()
add_subdirectory(src bin)
add_subdirectory(test)
