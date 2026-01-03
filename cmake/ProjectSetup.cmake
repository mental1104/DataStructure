include(CTest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_WERROR "Treat compiler warnings as errors" ON)
add_library(project_warnings INTERFACE)
if(ENABLE_WERROR)
    if(MSVC)
        target_compile_options(project_warnings INTERFACE /W4 /WX)
    else()
        target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic -Werror)
    endif()
endif()

if(APPLE)
    cmake_policy(SET CMP0068 NEW)
endif()

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install path" FORCE)
endif()
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

option(FORCE_STATIC_THIRD_PARTY "Force static build for thirdparty libs (googletest)" ON)
if(FORCE_STATIC_THIRD_PARTY)
    # 这个设置会影响 add_library 没有显式指定 SHARED/STATIC 的目标
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build libraries as static by default" FORCE)
endif()
