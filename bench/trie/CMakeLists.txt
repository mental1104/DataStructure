set(TRIE_BENCH_TARGET bench_trie_compare)

add_executable(${TRIE_BENCH_TARGET}
    bench_trie_compare.cpp
)

target_include_directories(${TRIE_BENCH_TARGET}
    PRIVATE
        ${INCLUDE_ROOT}
)

if(TARGET MyProjectLib)
    target_link_libraries(${TRIE_BENCH_TARGET} PRIVATE MyProjectLib)
endif()
