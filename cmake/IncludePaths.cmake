set(INCLUDE_ROOT ${CMAKE_SOURCE_DIR}/src/include)

# New namespaced headers are included from the include root, for example:
# #include <dsa/core/tree/BinTreeAlgorithm.h>
include_directories("${INCLUDE_ROOT}")

# Keep the historical flat include paths so existing teaching code can continue
# to use includes such as "Vector.h" and "BinTree.h" during migration.
file(GLOB HEADER_DIRS RELATIVE ${INCLUDE_ROOT} "${INCLUDE_ROOT}/*")
foreach(DIR ${HEADER_DIRS})
    if(IS_DIRECTORY "${INCLUDE_ROOT}/${DIR}")
        include_directories("${INCLUDE_ROOT}/${DIR}")
    endif()
endforeach()
