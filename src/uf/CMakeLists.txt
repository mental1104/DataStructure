include_directories(${TOP_DIR}/src/lib/utils)

file(COPY ${PROJECT_SOURCE_DIR}/src/uf/dataset 
     DESTINATION ${PROJECT_BINARY_DIR}/bin/uf/)

add_executable(QuickFind QuickFind.cpp)