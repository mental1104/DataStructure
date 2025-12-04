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
    find_program(GCOVR_EXEC gcovr)
    if(GCOVR_EXEC)
        add_custom_target(coverage
            COMMAND ${CMAKE_COMMAND} -E echo "[info] 清理生成目录下的旧 gcda（仅保留测试覆盖率）"
            COMMAND find ${CMAKE_BINARY_DIR}/CMakeFiles -name "*.gcda" -delete
            COMMAND ${CMAKE_COMMAND} -E echo "[info] 运行 ctest 生成覆盖率数据（排除 bench）"
            COMMAND ctest --output-on-failure -LE bench
            COMMAND ${GCOVR_EXEC}
                -r ${CMAKE_SOURCE_DIR}
                --object-directory ${CMAKE_BINARY_DIR}
                --filter '${CMAKE_SOURCE_DIR}/src/include'
                --exclude '(^|.*/)(test|bench|demo|external|gtest|lib|thirdparty|overlay)/'
                --exclude '/usr/include/.*'
                --exclude-directories '.*/build-(asan|tsan|ubsan|msan).*'
                --exclude-directories '.*/CMakeFiles/.*'
                --gcov-exclude '.*CMakeFiles/.*'
                --gcov-ignore-parse-errors
                --gcov-ignore-errors source_not_found
                --txt
                --print-summary
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Coverage summary (gcovr; excludes test/bench/thirdparty/lib/overlay)"
        )
    else()
        message(WARNING "gcovr not found; coverage target will be unavailable")
    endif()
endif()
