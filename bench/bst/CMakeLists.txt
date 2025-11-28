set(BST_BENCH_TARGET bst_bench)

add_executable(${BST_BENCH_TARGET}
    bst_bench.cpp
)

target_include_directories(${BST_BENCH_TARGET}
    PRIVATE
        ${INCLUDE_ROOT}
)

if(TARGET MyProjectLib)
    target_link_libraries(${BST_BENCH_TARGET} PRIVATE MyProjectLib)
endif()

if(BUILD_TESTING)
    add_test(
        NAME bst_bench
        COMMAND $<TARGET_FILE:${BST_BENCH_TARGET}>
    )
    set_tests_properties(bst_bench PROPERTIES LABELS "benchmark;bst")
endif()
