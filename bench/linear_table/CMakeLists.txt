set(LINEAR_BENCH_TARGET linear_table_bench)

add_executable(${LINEAR_BENCH_TARGET}
    linear_table_bench.cpp
)

target_include_directories(${LINEAR_BENCH_TARGET}
    PRIVATE
        ${INCLUDE_ROOT}
)

if(TARGET MyProjectLib)
    target_link_libraries(${LINEAR_BENCH_TARGET} PRIVATE MyProjectLib)
endif()

if(BUILD_TESTING)
    add_test(
        NAME linear_table_bench
        COMMAND $<TARGET_FILE:${LINEAR_BENCH_TARGET}>
            --size 20000
            --access-ops 50000
            --insert-ops 20000
            --seed 42
    )
    set_tests_properties(linear_table_bench PROPERTIES LABELS "benchmark;linear_table")
endif()
