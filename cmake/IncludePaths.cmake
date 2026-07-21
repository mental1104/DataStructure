set(SRC_ROOT ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_ROOT ${SRC_ROOT})
set(TUTORIAL_ROOT ${SRC_ROOT}/tutorials)

# 工业版使用 <dsa/...>，教学版真实实现使用 <tutorials/...>。
include_directories("${SRC_ROOT}")
include_directories("${TUTORIAL_ROOT}")

# 保留旧教学代码依赖的家族路径和短 include（如 <vector/Vector.h>、<Vector.h>）。
file(GLOB TUTORIAL_HEADER_DIRS RELATIVE ${TUTORIAL_ROOT} "${TUTORIAL_ROOT}/*")
foreach(DIR ${TUTORIAL_HEADER_DIRS})
    if(IS_DIRECTORY "${TUTORIAL_ROOT}/${DIR}")
        include_directories("${TUTORIAL_ROOT}/${DIR}")
    endif()
endforeach()
