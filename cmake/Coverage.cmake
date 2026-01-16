if(WIN32)
    set(COVERAGE_DEFAULT OFF)
else()
    set(COVERAGE_DEFAULT ON)
endif()

# Keep ENABLE_COVERAGE for compatibility; COVERAGE controls the behavior.
option(ENABLE_COVERAGE "Use gcov-style coverage flags" ${COVERAGE_DEFAULT})
option(COVERAGE "Enable coverage reporting" ${ENABLE_COVERAGE})

if(NOT WIN32)
    set(ENABLE_COVERAGE ON CACHE BOOL "Use gcov-style coverage flags" FORCE)
    set(COVERAGE ON CACHE BOOL "Enable coverage reporting" FORCE)
endif()

message(STATUS "ENABLE_COVERAGE=${ENABLE_COVERAGE}")
message(STATUS "COVERAGE=${COVERAGE}")

if(COVERAGE AND WIN32)
    message(WARNING "Coverage requested but Windows does not support gcov-style flags; disabling coverage.")
    set(COVERAGE OFF CACHE BOOL "Enable coverage reporting" FORCE)
endif()

if(COVERAGE)
    set(COVERAGE_SUPPORTED_COMPILER OFF)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(COVERAGE_SUPPORTED_COMPILER ON)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(COVERAGE_SUPPORTED_COMPILER ON)
    endif()

    if(COVERAGE_SUPPORTED_COMPILER)
        message(STATUS "Building with coverage support")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    else()
        message(WARNING "Coverage requested but unsupported compiler (${CMAKE_CXX_COMPILER_ID}); disabling coverage.")
        set(COVERAGE OFF CACHE BOOL "Enable coverage reporting" FORCE)
    endif()
endif()

if(COVERAGE)
    find_program(GCOVR_EXEC gcovr)

    set(GCOVR_GCOV_EXECUTABLE "")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        find_program(LLVM_COV_EXEC llvm-cov)
        if(NOT LLVM_COV_EXEC AND APPLE)
            find_program(XCRUN_EXEC xcrun)
            if(XCRUN_EXEC)
                execute_process(
                    COMMAND ${XCRUN_EXEC} --find llvm-cov
                    OUTPUT_VARIABLE LLVM_COV_EXEC
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET
                )
            endif()
        endif()
        if(LLVM_COV_EXEC AND EXISTS "${LLVM_COV_EXEC}")
            set(GCOVR_GCOV_EXECUTABLE "${LLVM_COV_EXEC} gcov")
        else()
            message(WARNING "llvm-cov not found; gcovr may fail for Clang coverage.")
        endif()
    endif()

    if(GCOVR_EXEC)
        if(GCOVR_GCOV_EXECUTABLE)
            add_custom_target(coverage
                COMMAND ${CMAKE_COMMAND} -E echo "[info] 清理生成目录下的旧 gcda（仅保留测试覆盖率）"
                COMMAND find ${CMAKE_BINARY_DIR}/CMakeFiles -name "*.gcda" -delete
                COMMAND ${CMAKE_COMMAND} -E echo "[info] 运行 ctest 生成覆盖率数据（排除 bench）"
                COMMAND ctest --output-on-failure -LE bench
                COMMAND ${GCOVR_EXEC}
                    --gcov-executable "${GCOVR_GCOV_EXECUTABLE}"
                    -r ${CMAKE_SOURCE_DIR}
                    --object-directory ${CMAKE_BINARY_DIR}
                    --filter "${CMAKE_SOURCE_DIR}/src/include"
                    --exclude "\\(^\\|.\\*/\\)\\(test\\|bench\\|demo\\|external\\|gtest\\|lib\\|thirdparty\\|overlay\\)/"
                    --exclude "${CMAKE_SOURCE_DIR}/src/include/print/.\\*"
                    --exclude "/usr/include/.\\*"
                    --exclude-directories ".\\*/build-\\(asan\\|tsan\\|ubsan\\|msan\\).\\*"
                    --exclude-directories ".\\*/CMakeFiles/.\\*"
                    --gcov-exclude ".\\*CMakeFiles/.\\*"
                    --gcov-ignore-parse-errors
                    --gcov-ignore-errors source_not_found
                    --merge-mode-functions merge
                    --xml-pretty -o "${CMAKE_SOURCE_DIR}/coverage.xml"
                    --print-summary
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Coverage summary (gcovr; excludes test/bench/thirdparty/lib/overlay)"
            )
        else()
            add_custom_target(coverage
                COMMAND ${CMAKE_COMMAND} -E echo "[info] 清理生成目录下的旧 gcda（仅保留测试覆盖率）"
                COMMAND find ${CMAKE_BINARY_DIR}/CMakeFiles -name "*.gcda" -delete
                COMMAND ${CMAKE_COMMAND} -E echo "[info] 运行 ctest 生成覆盖率数据（排除 bench）"
                COMMAND ctest --output-on-failure -LE bench
                COMMAND ${GCOVR_EXEC}
                    -r ${CMAKE_SOURCE_DIR}
                    --object-directory ${CMAKE_BINARY_DIR}
                    --filter "${CMAKE_SOURCE_DIR}/src/include"
                    --exclude "\\(^\\|.\\*/\\)\\(test\\|bench\\|demo\\|external\\|gtest\\|lib\\|thirdparty\\|overlay\\)/"
                    --exclude "${CMAKE_SOURCE_DIR}/src/include/print/.\\*"
                    --exclude "/usr/include/.\\*"
                    --exclude-directories ".\\*/build-\\(asan\\|tsan\\|ubsan\\|msan\\).\\*"
                    --exclude-directories ".\\*/CMakeFiles/.\\*"
                    --gcov-exclude ".\\*CMakeFiles/.\\*"
                    --gcov-ignore-parse-errors
                    --gcov-ignore-errors source_not_found
                    --merge-mode-functions merge
                    --xml-pretty -o "${CMAKE_SOURCE_DIR}/coverage.xml"
                    --print-summary
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Coverage summary (gcovr; excludes test/bench/thirdparty/lib/overlay)"
            )
        endif()
    else()
        message(WARNING "gcovr not found; coverage target will be unavailable")
    endif()
endif()
