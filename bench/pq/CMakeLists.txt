set(PQ_BENCH_TARGET pq_bench)

add_executable(${PQ_BENCH_TARGET}
    pq_bench.cpp
)

target_include_directories(${PQ_BENCH_TARGET}
    PRIVATE
        ${INCLUDE_ROOT}
)

target_link_libraries(${PQ_BENCH_TARGET} PRIVATE project_warnings)

if(TARGET MyProjectLib)
    target_link_libraries(${PQ_BENCH_TARGET} PRIVATE MyProjectLib)
endif()
