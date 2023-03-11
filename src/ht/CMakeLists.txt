include_directories(${TOP_DIR}/src/lib/utils)
include_directories(${TOP_DIR}/src/lib/entry)
include_directories(${TOP_DIR}/src/lib/print)
include_directories(${TOP_DIR}/src/list)
include_directories(${TOP_DIR}/src/bitmap)


add_executable(Skiplist Skiplist.cpp)
add_executable(Hashtable Hashtable.cpp)
add_executable(HashtableB HashtableB.cpp)