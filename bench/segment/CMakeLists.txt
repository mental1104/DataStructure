set(SEGMENT_BENCH_TARGET bench_segment_tree)

add_executable(${SEGMENT_BENCH_TARGET}
    bench_segment_tree.cpp
)

target_include_directories(${SEGMENT_BENCH_TARGET}
    PRIVATE
        ${INCLUDE_ROOT}
)

target_link_libraries(${SEGMENT_BENCH_TARGET} PRIVATE project_warnings)

if(TARGET MyProjectLib)
    target_link_libraries(${SEGMENT_BENCH_TARGET} PRIVATE MyProjectLib)
endif()
