set(SRC_ROOT ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_ROOT ${SRC_ROOT}/include)
set(TUTORIAL_ROOT ${SRC_ROOT}/tutorials)

# 工业版使用 <dsa/...>，教学版真实实现使用 <tutorials/...>。
include_directories("${INCLUDE_ROOT}")
include_directories("${SRC_ROOT}")

# 保留旧教学代码依赖的短 include（如 <Vector.h>、<List.h>）。
file(GLOB HEADER_DIRS RELATIVE ${INCLUDE_ROOT} "${INCLUDE_ROOT}/*")
foreach(DIR ${HEADER_DIRS})
    if(IS_DIRECTORY "${INCLUDE_ROOT}/${DIR}")
        include_directories("${INCLUDE_ROOT}/${DIR}")
    endif()
endforeach()
