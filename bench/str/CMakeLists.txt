set(STR_BENCH_TARGET bench_substring_match)
add_executable(${STR_BENCH_TARGET}
    bench_substring_match.cpp
)

add_executable(bench_suffix_array
    bench_suffix_array.cpp
)

target_include_directories(${STR_BENCH_TARGET}
    PRIVATE
        ${INCLUDE_ROOT}
)

target_include_directories(bench_suffix_array
    PRIVATE
        ${INCLUDE_ROOT}
)

target_link_libraries(${STR_BENCH_TARGET} PRIVATE project_warnings)
target_link_libraries(bench_suffix_array PRIVATE project_warnings)

if(TARGET MyProjectLib)
    target_link_libraries(${STR_BENCH_TARGET} PRIVATE MyProjectLib)
    target_link_libraries(bench_suffix_array PRIVATE MyProjectLib)
endif()
