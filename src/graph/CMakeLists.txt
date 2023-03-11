include_directories(${TOP_DIR}/src/lib/utils)
include_directories(${TOP_DIR}/src/dp)
include_directories(${TOP_DIR}/src/vector)
include_directories(${TOP_DIR}/src/list)
include_directories(${TOP_DIR}/src/stack)
include_directories(${TOP_DIR}/src/queue)
include_directories(${TOP_DIR}/src/pq)
include_directories(${TOP_DIR}/src/uf)


file(COPY ${PROJECT_SOURCE_DIR}/src/graph/dataset
     DESTINATION ${PROJECT_BINARY_DIR}/bin/graph/)

add_executable(GraphMatrix GraphMatrix.cpp)