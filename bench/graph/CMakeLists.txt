set(GRAPH_BENCH_TARGET bench_graph_compare)

add_executable(${GRAPH_BENCH_TARGET}
    bench_graph.cpp
)

target_include_directories(${GRAPH_BENCH_TARGET}
    PRIVATE
        ${INCLUDE_ROOT}
)

target_link_libraries(${GRAPH_BENCH_TARGET} PRIVATE project_warnings)

if(TARGET MyProjectLib)
    target_link_libraries(${GRAPH_BENCH_TARGET} PRIVATE MyProjectLib)
endif()
