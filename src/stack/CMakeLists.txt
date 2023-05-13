file(COPY ${PROJECT_SOURCE_DIR}/src/stack/laby 
     DESTINATION ${PROJECT_BINARY_DIR}/bin/stack/)

add_executable(Stack Stack.cpp)
add_executable(Queen Queen.cpp)
add_executable(Convert convert.cpp)
add_executable(Evaluate evaluate.cpp)
target_compile_options(Evaluate PRIVATE -Wformat)
add_executable(Labyrinth labyrinth.cpp)
add_executable(Paren paren.cpp)
