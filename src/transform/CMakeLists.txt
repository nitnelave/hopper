target_sources(${GRACC_LIBRARY}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/function_value_body.cc"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/function_value_body.h"
    )
