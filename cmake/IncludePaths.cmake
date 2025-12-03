set(INCLUDE_ROOT ${CMAKE_SOURCE_DIR}/src/include)

# 遍历 include 目录下的所有子目录并加入头文件路径
file(GLOB HEADER_DIRS RELATIVE ${INCLUDE_ROOT} "${INCLUDE_ROOT}/*")
foreach(DIR ${HEADER_DIRS})
    if(IS_DIRECTORY "${INCLUDE_ROOT}/${DIR}")
        include_directories("${INCLUDE_ROOT}/${DIR}")
    endif()
endforeach()
