cmake_minimum_required(VERSION 3.2.0)
project(DSA CXX)

# 设置编译选项
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置根目录
set(INCLUDE_ROOT ${CMAKE_SOURCE_DIR}/src/include)

if(EXISTS "${CMAKE_SOURCE_DIR}/demo")
    # 确保构建目录中的 demo 基础目录存在
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/demo")
    # 获取 demo 下所有文件（包括子目录中的文件）
    file(GLOB_RECURSE DEMO_FILES RELATIVE "${CMAKE_SOURCE_DIR}/demo" "${CMAKE_SOURCE_DIR}/demo/*")
    set(DEMO_DIRS "")
    foreach(ITEM ${DEMO_FILES})
        # 提取 ITEM 的父目录
        get_filename_component(DIR ${ITEM} DIRECTORY)
        # 如果结果不是当前目录，则加入列表
        if(NOT DIR STREQUAL ".")
            list(APPEND DEMO_DIRS ${DIR})
        endif()
    endforeach()
    # 去重，防止重复创建同一个目录
    list(REMOVE_DUPLICATES DEMO_DIRS)
    foreach(D ${DEMO_DIRS})
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/demo/${D}")
        message(STATUS "Created directory: ${CMAKE_BINARY_DIR}/demo/${D}")
    endforeach()
endif()


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

# 查找 demo 目录下的所有 .py 文件
file(GLOB_RECURSE DEMO_PY_FILES_PY "${CMAKE_SOURCE_DIR}/demo/**/*.py")
# 查找 demo 目录下的所有 .txt 文件
file(GLOB_RECURSE DEMO_PY_FILES_TXT  "${CMAKE_SOURCE_DIR}/demo/**/*.txt")
set(DEMO_PY_FILES ${DEMO_PY_FILES_PY} ${DEMO_PY_FILES_TXT})

if(DEMO_PY_FILES)
    foreach(PY_FILE ${DEMO_PY_FILES})
        # 计算相对路径
        file(RELATIVE_PATH RELATIVE_PATH_TO_DEMO "${CMAKE_SOURCE_DIR}/demo" ${PY_FILE})

        # 计算目标路径
        set(TARGET_PY_FILE "${CMAKE_BINARY_DIR}/demo/${RELATIVE_PATH_TO_DEMO}")

        # 确保目标目录存在
        get_filename_component(TARGET_PY_DIR "${TARGET_PY_FILE}" DIRECTORY)
        file(MAKE_DIRECTORY ${TARGET_PY_DIR})

        # 复制 Python 文件
        file(COPY ${PY_FILE} DESTINATION ${TARGET_PY_DIR})

        message(STATUS "Copied ${PY_FILE} to ${TARGET_PY_DIR}")
    endforeach()
else()
    message(STATUS "No Python files found in demo directory.")
endif()


# ========== 新增：编译 bst.so 共享库 ==========
# 查找 bst 相关的源文件
set(LIBRARY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/lib")
set(ENV{LD_LIBRARY_PATH} "${LIBRARY_OUTPUT_DIR}:$ENV{LD_LIBRARY_PATH}")
message(STATUS "LD_LIBRARY_PATH is set to: $ENV{LD_LIBRARY_PATH}")

file(GLOB BST_SRC_FILES "${CMAKE_SOURCE_DIR}/src/lib/bst_api.cpp")

if(BST_SRC_FILES)
    add_library(bst SHARED ${BST_SRC_FILES})

    # 设置 include 目录
    target_include_directories(bst PUBLIC ${INCLUDE_ROOT})
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

    # 指定 bst.so 的输出路径
    set_target_properties(bst PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )

    message(STATUS "Built shared library: bst.so in ${CMAKE_BINARY_DIR}/lib")
else()
    message(WARNING "No BST source files found. Skipping bst.so compilation.")
endif()


file(GLOB SORT_SRC_FILES "${CMAKE_SOURCE_DIR}/src/lib/sort_api.cpp")

if(SORT_SRC_FILES)
    add_library(sort SHARED ${SORT_SRC_FILES})

    # 设置 include 目录
    target_include_directories(sort PUBLIC ${INCLUDE_ROOT})
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

    # 指定 sort.so 的输出路径
    set_target_properties(sort PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )

    message(STATUS "Built shared library: sort.so in ${CMAKE_BINARY_DIR}/lib")
else()
    message(WARNING "No SORT source files found. Skipping sort.so compilation.")
endif()

# 检查 demo 目录是否存在且包含 .cpp 文件
if(EXISTS "${CMAKE_SOURCE_DIR}/demo")

    # 递归查找 demo 目录下的所有 .cpp 文件
    file(GLOB_RECURSE DEMO_FILES "${CMAKE_SOURCE_DIR}/demo/**/*.cpp")
    
    if(DEMO_FILES)
        foreach(CPP_FILE ${DEMO_FILES})
            # 获取文件相对于 demo 的路径
            file(RELATIVE_PATH RELATIVE_PATH_TO_DEMO "${CMAKE_SOURCE_DIR}/demo" ${CPP_FILE})
            
            # 提取相对路径的目录部分
            get_filename_component(RELATIVE_DIR "${RELATIVE_PATH_TO_DEMO}" DIRECTORY)
            
            # 提取文件名（不含扩展名）
            get_filename_component(EXECUTABLE_NAME ${CPP_FILE} NAME_WE)
            
            # 判断是否以 test_ 开头，如果是，则输出目录改为 ${CMAKE_BINARY_DIR}/bin，
            # 否则放到 build/demo/<子目录> 下
            if("${EXECUTABLE_NAME}" MATCHES "^test_")
                set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
            else()
                set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/demo/${RELATIVE_DIR}")
            endif()
            
            # 确保输出目录存在
            file(MAKE_DIRECTORY "${OUTPUT_DIR}")
            
            # 添加可执行文件
            add_executable(${EXECUTABLE_NAME} ${CPP_FILE})
            
            # 仅为此目标设置运行时输出目录，不使用全局变量
            set_target_properties(${EXECUTABLE_NAME} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
            )
            
            # 如果需要链接库，添加链接
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


# 检查 thirdparty 目录是否存在且包含 googletest
set(GTEST_DIR "${CMAKE_SOURCE_DIR}/thirdparty/googletest")

# 检查 thirdparty 目录是否为空
if(EXISTS "${GTEST_DIR}" AND IS_DIRECTORY "${GTEST_DIR}")
    file(GLOB GTEST_FILES "${GTEST_DIR}/*")
    list(LENGTH GTEST_FILES GTEST_COUNT)

    if(GTEST_COUNT GREATER 0)
        # 只有当 googletest 目录非空时才启用测试
        enable_testing()
        add_subdirectory(thirdparty/googletest)

        # 设置测试目录
        set(TEST_DIR "${CMAKE_SOURCE_DIR}/test")

        if(EXISTS "${TEST_DIR}")
            file(GLOB_RECURSE TEST_SOURCES "${TEST_DIR}/*.cpp")

            if(TEST_SOURCES)
                message(STATUS "Found test files in ${TEST_DIR}")

                foreach(TEST_FILE ${TEST_SOURCES})
                    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
                    add_executable(${TEST_NAME} ${TEST_FILE})
                    target_link_libraries(${TEST_NAME} PRIVATE gtest gtest_main pthread)
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
