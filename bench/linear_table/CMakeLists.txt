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

