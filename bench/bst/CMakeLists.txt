set(BST_BENCH_TARGET ds_bench)

add_executable(${BST_BENCH_TARGET}
    ds_bench.cpp
)

target_include_directories(${BST_BENCH_TARGET}
    PRIVATE
        ${INCLUDE_ROOT}
)

target_link_libraries(${BST_BENCH_TARGET} PRIVATE project_warnings)

if(TARGET MyProjectLib)
    target_link_libraries(${BST_BENCH_TARGET} PRIVATE MyProjectLib)
endif()
