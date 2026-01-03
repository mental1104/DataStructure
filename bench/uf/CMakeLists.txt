set(UF_BENCH_TARGET bench_uf_compare)

add_executable(${UF_BENCH_TARGET}
    bench_uf.cpp
)

target_include_directories(${UF_BENCH_TARGET}
    PRIVATE
        ${INCLUDE_ROOT}
)

target_link_libraries(${UF_BENCH_TARGET} PRIVATE project_warnings)

if(TARGET MyProjectLib)
    target_link_libraries(${UF_BENCH_TARGET} PRIVATE MyProjectLib)
endif()
