include_directories(${TOP_DIR}/src/common/lib/utils)
include_directories(${TOP_DIR}/src/common/dp)
include_directories(${TOP_DIR}/src/common/vector)
include_directories(${TOP_DIR}/src/common/list)

file(COPY ${PROJECT_SOURCE_DIR}/src/common/stack/laby 
     DESTINATION ${PROJECT_BINARY_DIR}/bin/common/stack/)

add_executable(Stack Stack.cpp)
add_executable(Queen Queen.cpp)
add_executable(Convert convert.cpp)
add_executable(Evaluate evaluate.cpp)
add_executable(Labyrinth labyrinth.cpp)
add_executable(Paren paren.cpp)
