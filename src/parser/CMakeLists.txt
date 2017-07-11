target_sources(${GRACC_LIBRARY}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/parser.cc"
        "${CMAKE_CURRENT_LIST_DIR}/scoped_location.cc"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/parser.h"
        "${CMAKE_CURRENT_LIST_DIR}/scoped_location.h"
    )
