target_sources(${GRACC_LIBRARY}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/logging.cc"
        "${CMAKE_CURRENT_LIST_DIR}/gflags_utils.cc"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/gflags_utils.h"
        "${CMAKE_CURRENT_LIST_DIR}/logging.h"
        "${CMAKE_CURRENT_LIST_DIR}/lookahead_stack.h"
        "${CMAKE_CURRENT_LIST_DIR}/option.h"
        "${CMAKE_CURRENT_LIST_DIR}/variant.h"
    )
