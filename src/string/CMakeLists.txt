include_directories(${TOP_DIR}/src/lib/utils)
include_directories(${TOP_DIR}/src/dp)
include_directories(${TOP_DIR}/src/vector)
include_directories(${TOP_DIR}/src/list)
include_directories(${TOP_DIR}/src/queue)

add_executable(String String.cpp)
add_executable(Trie Trie.cpp)
add_executable(TST TST.cpp)