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
            --size 300000
            --access-ops 1000000
            --locality-ops 1000000
            --insert-ops 500000
            --erase-ops 500000
            --seed 42
    )
    set_tests_properties(bst_bench PROPERTIES LABELS "benchmark;bst")
endif()
