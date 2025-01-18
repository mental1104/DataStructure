cmake_minimum_required(VERSION 3.2.0)
project(DSA CXX)

# 设置编译选项
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置根目录
set(INCLUDE_ROOT ${CMAKE_SOURCE_DIR}/src/include)

# 遍历 include 目录下的所有子目录并加入头文件路径
file(GLOB HEADER_DIRS ${INCLUDE_ROOT}/*)
foreach(DIR ${HEADER_DIRS})
    message(${DIR})
    if(IS_DIRECTORY ${DIR})
        include_directories(${DIR})
    endif()
endforeach()


# 查找 src 目录下的所有源文件并创建一个库（如果有文件）
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")
if(SRC_FILES) # 如果 src 目录中有源文件
    add_library(MyProjectLib ${SRC_FILES})
else()
    message(STATUS "No source files found in src directory. Skipping MyProjectLib.")
endif()


# 检查 demo 目录是否存在且包含 .cpp 文件
if(EXISTS "${CMAKE_SOURCE_DIR}/demo")
    # 递归查找 demo 目录下的所有 .cpp 文件
    file(GLOB_RECURSE DEMO_FILES "${CMAKE_SOURCE_DIR}/demo/**/*.cpp")
    
    if(DEMO_FILES)
        foreach(CPP_FILE ${DEMO_FILES})
            # 获取文件相对于 demo 的路径（不含根目录）
            file(RELATIVE_PATH RELATIVE_PATH_TO_DEMO "${CMAKE_SOURCE_DIR}/demo" ${CPP_FILE})
            
            # 提取相对路径的目录部分
            get_filename_component(RELATIVE_DIR "${RELATIVE_PATH_TO_DEMO}" DIRECTORY)
            
            # 提取文件名（不含扩展名）
            get_filename_component(EXECUTABLE_NAME ${CPP_FILE} NAME_WE)
            
            # 设置输出目录为 build/demo/<相对路径的目录>
            set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/demo/${RELATIVE_DIR}")
            
            # 创建目录（确保路径存在）
            file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
            
            # 添加可执行文件
            add_executable(${EXECUTABLE_NAME} ${CPP_FILE})
            
            # 设置目标的输出目录（仅目录部分，去掉多余的子目录）
            set_target_properties(${EXECUTABLE_NAME} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}"
            )
            
            # 如果需要链接库，添加链接
            if(TARGET MyProjectLib)
                target_link_libraries(${EXECUTABLE_NAME} PRIVATE MyProjectLib)
            endif()
            
            # 输出生成的可执行文件信息
            message(STATUS "Created executable: ${EXECUTABLE_NAME} from ${CPP_FILE}, output to ${EXECUTABLE_OUTPUT_PATH}")
        endforeach()
    else()
        message(STATUS "demo directory exists but contains no .cpp files. No executables will be built.")
    endif()
else()
    message(STATUS "demo directory does not exist. No executables will be built.")
endif()


# 添加测试所需的 CTest
enable_testing()

# 设置测试目录
set(TEST_DIR "${CMAKE_SOURCE_DIR}/test")

# 检查目录是否存在且包含 .cpp 文件
if (EXISTS "${TEST_DIR}")
    # 查找 .cpp 文件
    file(GLOB_RECURSE TEST_SOURCES "${TEST_DIR}/*.cpp")
    
    # 如果找到文件则处理
    if (TEST_SOURCES)
        message(STATUS "Found test files in ${TEST_DIR}: ${TEST_SOURCES}")

        # 遍历每个 .cpp 文件，将其添加为单独的测试
        foreach(TEST_FILE ${TEST_SOURCES})
            # 获取文件名（不带路径和扩展名）
            get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

            # 创建测试的目标
            add_executable(${TEST_NAME} ${TEST_FILE})

            # 链接测试所需的库（如 GTest）
            target_link_libraries(${TEST_NAME} PRIVATE gtest gtest_main pthread)

            # 添加到测试集中
            add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        endforeach()
    else()
        message(WARNING "Test directory exists but no .cpp files were found in ${TEST_DIR}.")
    endif()
else()
    message(WARNING "Test directory ${TEST_DIR} does not exist.")
endif()

# 开启覆盖率
OPTION (ENABLE_COVERAGE "Use gcov" ON)
MESSAGE(STATUS ENABLE_COVERAGE=${ENABLE_COVERAGE})
IF(ENABLE_COVERAGE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF()

# 启用覆盖率选项
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


# 设置 CMake 安装前缀
set(CMAKE_INSTALL_PREFIX "/usr/local")

# 遍历所有子目录的 .h 文件
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/src/include/*.h)

# 安装所有头文件到 DSA 目录，不保留目录结构
install(FILES ${HEADER_FILES}
        DESTINATION include/DSA)
