file(COPY ${PROJECT_SOURCE_DIR}/src/graph/dataset
     DESTINATION ${PROJECT_BINARY_DIR}/bin/graph/)

add_executable(GraphMatrix GraphMatrix.cpp)