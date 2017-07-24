target_sources(${GRACC_LIBRARY}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/typechecker.cc"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/typechecker.h"
    )
