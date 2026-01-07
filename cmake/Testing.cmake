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
                    target_link_libraries(${TEST_NAME} PRIVATE project_warnings gtest gtest_main Threads::Threads)

                    # Suppress Clang's C++17 attribute-extension warning triggered by gtest macros in tests.
                    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                        target_compile_options(${TEST_NAME} PRIVATE
                            -Wno-c++17-attribute-extensions
                            -Wno-error=c++17-attribute-extensions
                            -Wno-keyword-macro
                            -Wno-error=keyword-macro
                        )
                    endif()

                    # 为了解决 BPlusTree.h 的最后一行仍未覆盖的问题，test_bplus_tree 在编译时禁用拷贝消除，这样 rangeQuery 的返回收尾行会被 gcov 记到；之前新增的高区间 rangeQuery 用例继续保证走到末尾 return res
                    if ((TEST_NAME STREQUAL "test_bplus_tree" OR TEST_NAME STREQUAL "test_string" OR TEST_NAME STREQUAL "test_trie" OR TEST_NAME STREQUAL "test_vector")
                        AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
                        target_compile_options(${TEST_NAME} PRIVATE -fno-elide-constructors)
                    endif()

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
