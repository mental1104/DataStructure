cmake_minimum_required(VERSION 3.2.0)
project(DSA CXX)

# ------------------ 基本配置 ------------------
include(CTest)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
    cmake_policy(SET CMP0068 NEW)
endif()

# 默认把安装前缀设到用户目录（可通过 -DCMAKE_INSTALL_PREFIX=... 覆盖）
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install path" FORCE)
endif()
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

# 可选：强制第三方库静态构建以避免运行时找不到 .so 的问题
option(FORCE_STATIC_THIRD_PARTY "Force static build for thirdparty libs (googletest)" ON)
if(FORCE_STATIC_THIRD_PARTY)
    # 这个设置会影响 add_library 没有显式指定 SHARED/STATIC 的目标
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build libraries as static by default" FORCE)
endif()

# ------------------ 路径和包含 ------------------
set(INCLUDE_ROOT ${CMAKE_SOURCE_DIR}/src/include)

# 遍历 include 目录下的所有子目录并加入头文件路径
file(GLOB HEADER_DIRS RELATIVE ${INCLUDE_ROOT} "${INCLUDE_ROOT}/*")
foreach(DIR ${HEADER_DIRS})
    if(IS_DIRECTORY "${INCLUDE_ROOT}/${DIR}")
        include_directories("${INCLUDE_ROOT}/${DIR}")
    endif()
endforeach()

# ------------------ 项目库（可选） ------------------
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")
if(SRC_FILES)
    add_library(MyProjectLib ${SRC_FILES})
else()
    message(STATUS "No source files found in src directory. Skipping MyProjectLib.")
endif()

# ------------------ demo 文件复制（保持原逻辑） ------------------
file(GLOB_RECURSE DEMO_PY_FILES_PY "${CMAKE_SOURCE_DIR}/demo/**/*.py")
file(GLOB_RECURSE DEMO_PY_FILES_TXT "${CMAKE_SOURCE_DIR}/demo/**/*.txt")
set(DEMO_PY_FILES ${DEMO_PY_FILES_PY} ${DEMO_PY_FILES_TXT})
if(DEMO_PY_FILES)
    foreach(PY_FILE ${DEMO_PY_FILES})
        file(RELATIVE_PATH RELATIVE_PATH_TO_DEMO "${CMAKE_SOURCE_DIR}/demo" ${PY_FILE})
        set(TARGET_PY_FILE "${CMAKE_BINARY_DIR}/demo/${RELATIVE_PATH_TO_DEMO}")
        get_filename_component(TARGET_PY_DIR "${TARGET_PY_FILE}" DIRECTORY)
        file(MAKE_DIRECTORY ${TARGET_PY_DIR})
        file(COPY ${PY_FILE} DESTINATION ${TARGET_PY_DIR})
        message(STATUS "Copied ${PY_FILE} to ${TARGET_PY_DIR}")
    endforeach()
else()
    message(STATUS "No Python files found in demo directory.")
endif()

# ------------------ 构建共享库（bst/sort） ------------------
set(LIBRARY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/lib")
set(ENV{LD_LIBRARY_PATH} "${LIBRARY_OUTPUT_DIR}:$ENV{LD_LIBRARY_PATH}")
message(STATUS "LD_LIBRARY_PATH is set to: $ENV{LD_LIBRARY_PATH}")

file(GLOB BST_SRC_FILES "${CMAKE_SOURCE_DIR}/src/lib/bst_api.cpp")
if(BST_SRC_FILES)
    add_library(bstapi SHARED ${BST_SRC_FILES})
    target_include_directories(bstapi PUBLIC ${INCLUDE_ROOT})
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set_target_properties(bstapi PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    message(STATUS "Built shared library: bst.so in ${CMAKE_BINARY_DIR}/lib")
else()
    message(WARNING "No BST source files found. Skipping bst.so compilation.")
endif()

file(GLOB SORT_SRC_FILES "${CMAKE_SOURCE_DIR}/src/lib/sort_api.cpp")
if(SORT_SRC_FILES)
    add_library(sort SHARED ${SORT_SRC_FILES})
    target_include_directories(sort PUBLIC ${INCLUDE_ROOT})
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set_target_properties(sort PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    message(STATUS "Built shared library: sort.so in ${CMAKE_BINARY_DIR}/lib")
else()
    message(WARNING "No SORT source files found. Skipping sort.so compilation.")
endif()

# ------------------ demo/cpp 可执行文件（保持原逻辑） ------------------
if(EXISTS "${CMAKE_SOURCE_DIR}/demo")
    file(GLOB_RECURSE DEMO_FILES "${CMAKE_SOURCE_DIR}/demo/**/*.cpp")
    if(DEMO_FILES)
        foreach(CPP_FILE ${DEMO_FILES})
            file(RELATIVE_PATH RELATIVE_PATH_TO_DEMO "${CMAKE_SOURCE_DIR}/demo" ${CPP_FILE})
            get_filename_component(RELATIVE_DIR "${RELATIVE_PATH_TO_DEMO}" DIRECTORY)
            get_filename_component(EXECUTABLE_NAME ${CPP_FILE} NAME_WE)
            if("${EXECUTABLE_NAME}" MATCHES "^test_")
                set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
            else()
                set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/demo/${RELATIVE_DIR}")
            endif()
            file(MAKE_DIRECTORY "${OUTPUT_DIR}")
            add_executable(${EXECUTABLE_NAME} ${CPP_FILE})
            set_target_properties(${EXECUTABLE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
            if(TARGET MyProjectLib)
                target_link_libraries(${EXECUTABLE_NAME} PRIVATE MyProjectLib)
            endif()
            message(STATUS "Created executable: ${EXECUTABLE_NAME} from ${CPP_FILE}, output to ${OUTPUT_DIR}")
        endforeach()
    else()
        message(STATUS "demo directory exists but contains no .cpp files. No executables will be built.")
    endif()
else()
    message(STATUS "demo directory does not exist. No executables will be built.")
endif()

# ------------------ benchmarks ------------------
option(ENABLE_BENCHMARKS "Build benchmark executables" ON)
if(ENABLE_BENCHMARKS AND EXISTS "${CMAKE_SOURCE_DIR}/bench")
    add_subdirectory(bench)
else()
    message(STATUS "Benchmarks are disabled or bench directory missing; skipping benchmark targets.")
endif()

# ------------------ googletest & 单元测试 ------------------
set(GTEST_DIR "${CMAKE_SOURCE_DIR}/thirdparty/googletest")
if(EXISTS "${GTEST_DIR}" AND IS_DIRECTORY "${GTEST_DIR}")
    file(GLOB GTEST_FILES "${GTEST_DIR}/*")
    list(LENGTH GTEST_FILES GTEST_COUNT)
    if(GTEST_COUNT GREATER 0)
        enable_testing()
        add_subdirectory(thirdparty/googletest)

        # 确保可用的线程库
        find_package(Threads REQUIRED)

        set(TEST_DIR "${CMAKE_SOURCE_DIR}/test")
        if(EXISTS "${TEST_DIR}")
            file(GLOB_RECURSE TEST_SOURCES "${TEST_DIR}/*.cpp")
            if(TEST_SOURCES)
                message(STATUS "Found test files in ${TEST_DIR}")
                foreach(TEST_FILE ${TEST_SOURCES})
                    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
                    add_executable(${TEST_NAME} ${TEST_FILE})

                    # 链接 gtest、gtest_main、线程库
                    target_link_libraries(${TEST_NAME} PRIVATE gtest gtest_main Threads::Threads)

                    # 如果 googletest 是以库 target 形式存在，用 target 的输出目录作为测试的 RPATH
                    # 这样在未安装 gtest 的情况下，运行测试也能找到库
                    set_target_properties(${TEST_NAME} PROPERTIES
                        BUILD_RPATH "$<TARGET_FILE_DIR:gtest>"
                        INSTALL_RPATH "$<TARGET_FILE_DIR:gtest>"
                    )

                    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
                endforeach()
            else()
                message(WARNING "Test directory exists but no .cpp files found.")
            endif()
        else()
            message(WARNING "Test directory ${TEST_DIR} does not exist.")
        endif()
    else()
        message(STATUS "googletest directory exists but is empty. Tests will not be enabled.")
    endif()
else()
    message(STATUS "googletest directory does not exist. Tests will not be enabled.")
endif()

# ------------------ 覆盖率（保留原有逻辑） ------------------
OPTION (ENABLE_COVERAGE "Use gcov" ON)
MESSAGE(STATUS ENABLE_COVERAGE=${ENABLE_COVERAGE})
IF(ENABLE_COVERAGE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    option(COVERAGE "Enable coverage reporting" ON)
    if(COVERAGE)
        message(STATUS "Building with coverage support")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    endif()
endif()

if(COVERAGE)
    find_program(LCOV_EXEC lcov REQUIRED)
    find_program(GENHTML_EXEC genhtml REQUIRED)
    if(LCOV_EXEC AND GENHTML_EXEC)
        add_custom_target(coverage
            COMMAND ${LCOV_EXEC} --directory ${CMAKE_BINARY_DIR} --capture --ignore-errors inconsistent --output-file coverage.info
            COMMAND ${LCOV_EXEC} --ignore-errors unused --remove coverage.info '*/test/*' '/usr/*' '*/external/*' '*/gtest/*' --output-file coverage_filtered.info
            COMMAND ${GENHTML_EXEC} coverage_filtered.info --output-directory ${CMAKE_BINARY_DIR}/coverage_report
            COMMENT "Generating coverage report..."
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    else()
        message(WARNING "lcov or genhtml not found, coverage target will not work")
    endif()
endif()

# ------------------ 安装头文件（保留目录结构） ------------------
# 把 src/include/ 下的目录树安装到 include/DSA 下，保留结构
install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/include/
        DESTINATION include/DSA
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

# 结束
message(STATUS "CMake configuration finished.")
