set(STR_BENCH_TARGET bench_substring_match)

add_executable(${STR_BENCH_TARGET}
    bench_substring_match.cpp
)

target_include_directories(${STR_BENCH_TARGET}
    PRIVATE
        ${INCLUDE_ROOT}
)

if(TARGET MyProjectLib)
    target_link_libraries(${STR_BENCH_TARGET} PRIVATE MyProjectLib)
endif()
