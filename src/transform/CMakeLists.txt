target_sources(${GRACC_LIBRARY}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/function_value_body.cc"
        "${CMAKE_CURRENT_LIST_DIR}/add_return.cc"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/function_value_body.h"
        "${CMAKE_CURRENT_LIST_DIR}/add_return.h"
    )
