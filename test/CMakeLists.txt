# include_directories(${TOP_DIR}/src/common/dp)
# include_directories(${TOP_DIR}/src/common/vector)
include_directories()

add_executable(unit_tests                
                unit_tests.cpp) 

target_include_directories(unit_tests PRIVATE ${TOP_DIR}/src/dp)   
target_include_directories(unit_tests PRIVATE ${TOP_DIR}/src/lib/utils)
target_include_directories(unit_tests PRIVATE ${TOP_DIR}/src/vector) 
add_test(NAME testVectorFind COMMAND unit_tests 1) 
# add_test(NAME testVectorSearch COMMAND unit_tests 2)