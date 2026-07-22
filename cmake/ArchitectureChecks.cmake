find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_custom_target(check_dsa_stl_dependencies ALL
    COMMAND
        "${Python3_EXECUTABLE}"
        "${CMAKE_SOURCE_DIR}/tools/check_dsa_stl_dependencies.py"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Checking production DSA code for forbidden STL dependencies"
    VERBATIM
)
