install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/include/
        DESTINATION include/DSA
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/tutorials/
        DESTINATION include/DSA/tutorials
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
