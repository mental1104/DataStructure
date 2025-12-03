cmake_minimum_required(VERSION 3.2.0)
project(DSA CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(ProjectSetup)
include(IncludePaths)
include(CoreLibrary)
include(DemoCopy)
include(SharedLibraries)
include(DemoExecutables)
include(Benchmarks)
include(Testing)
include(Coverage)
include(InstallHeaders)

message(STATUS "CMake configuration finished.")
