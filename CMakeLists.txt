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

# # 检查 test 目录是否存在且包含 cpp 文件
# if(EXISTS "${CMAKE_SOURCE_DIR}/test")
#     file(GLOB TEST_FILES "${CMAKE_SOURCE_DIR}/test/*.cpp")
#     if(TEST_FILES)
#         add_executable(test_runner ${TEST_FILES})
#         if(TARGET MyProjectLib)
#             target_link_libraries(test_runner PRIVATE MyProjectLib)
#         endif()
#         message(STATUS "Test runner will be built.")
#     else()
#         message(STATUS "test directory exists but is empty. Skipping test_runner.")
#     endif()
# else()
#     message(STATUS "test directory does not exist. Skipping test_runner.")
# endif()

# 检查 demo 目录是否存在且包含 .cpp 文件

if(EXISTS "${CMAKE_SOURCE_DIR}/demo")
    # 递归查找 demo 目录下的所有 .cpp 文件
    file(GLOB_RECURSE DEMO_FILES "${CMAKE_SOURCE_DIR}/demo/**/*.cpp")
    
    if(DEMO_FILES)
        # 遍历每个找到的 .cpp 文件
        foreach(CPP_FILE ${DEMO_FILES})
            # 获取文件名（不带路径和扩展名）
            get_filename_component(EXECUTABLE_NAME ${CPP_FILE} NAME_WE)
            
            # 创建一个与文件名对应的可执行文件
            add_executable(${EXECUTABLE_NAME} ${CPP_FILE})
            
            # 如果需要链接库，添加链接
            if(TARGET MyProjectLib)
                target_link_libraries(${EXECUTABLE_NAME} PRIVATE MyProjectLib)
            endif()
            
            # 输出生成的可执行文件信息
            message(STATUS "Created executable: ${EXECUTABLE_NAME} from ${CPP_FILE}")
        endforeach()
    else()
        message(STATUS "demo directory exists but contains no .cpp files. No executables will be built.")
    endif()
else()
    message(STATUS "demo directory does not exist. No executables will be built.")
endif()
