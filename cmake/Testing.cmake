set(GTEST_DIR "${CMAKE_SOURCE_DIR}/thirdparty/googletest")
if(EXISTS "${GTEST_DIR}" AND IS_DIRECTORY "${GTEST_DIR}")
    file(GLOB GTEST_FILES "${GTEST_DIR}/*")
    list(LENGTH GTEST_FILES GTEST_COUNT)
    if(GTEST_COUNT GREATER 0)
        enable_testing()
        add_subdirectory(thirdparty/googletest)

        # Allow gtest headers to emit C++17-attribute warnings without failing the build
        # when compiling with C++11/14 on Clang-based toolchains.
        if (CMAKE_CXX_STANDARD LESS 17 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set(GTEST_NOERROR_FLAGS "-Wno-error=c++17-attribute-extensions")
            if (TARGET gtest)
                target_compile_options(gtest INTERFACE ${GTEST_NOERROR_FLAGS})
            endif()
            if (TARGET gtest_main)
                target_compile_options(gtest_main INTERFACE ${GTEST_NOERROR_FLAGS})
            endif()
            if (TARGET gmock)
                target_compile_options(gmock INTERFACE ${GTEST_NOERROR_FLAGS})
            endif()
            if (TARGET gmock_main)
                target_compile_options(gmock_main INTERFACE ${GTEST_NOERROR_FLAGS})
            endif()
        endif()

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
                    target_link_libraries(${TEST_NAME} PRIVATE project_warnings gtest gtest_main Threads::Threads)

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
