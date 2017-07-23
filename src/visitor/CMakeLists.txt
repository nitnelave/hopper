target_sources(${GRACC_LIBRARY}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/visitor.cc"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/error_visitor.h"
        "${CMAKE_CURRENT_LIST_DIR}/visitor.h"
    )
