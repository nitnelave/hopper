target_sources(${GRACC_LIBRARY}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/binary_operation.cc"
        "${CMAKE_CURRENT_LIST_DIR}/function_declaration.cc"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/pretty_printer.h"
    )
