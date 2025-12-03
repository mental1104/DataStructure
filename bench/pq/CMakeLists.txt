set(PQ_BENCH_TARGET pq_bench)

add_executable(${PQ_BENCH_TARGET}
    pq_bench.cpp
)

target_include_directories(${PQ_BENCH_TARGET}
    PRIVATE
        ${INCLUDE_ROOT}
)

if(TARGET MyProjectLib)
    target_link_libraries(${PQ_BENCH_TARGET} PRIVATE MyProjectLib)
endif()

if(BUILD_TESTING)
    add_test(
        NAME pq_bench
        COMMAND $<TARGET_FILE:${PQ_BENCH_TARGET}>
            --init 50000
            --mixed-ops 80000
            --meld-heaps 400
            --meld-heap-size 200
            --seed 42
    )
    set_tests_properties(pq_bench PROPERTIES LABELS "benchmark;pq")
endif()
