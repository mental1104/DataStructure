set(SORT_BENCH_TARGET sort_bench)

add_executable(${SORT_BENCH_TARGET}
    sort_bench.cpp
)

target_include_directories(${SORT_BENCH_TARGET}
    PRIVATE
        ${INCLUDE_ROOT}
)

if(TARGET MyProjectLib)
    target_link_libraries(${SORT_BENCH_TARGET} PRIVATE MyProjectLib)
endif()
