target_sources(${GRACC_LIBRARY}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/lexer.cc"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/lexer.h"
        "${CMAKE_CURRENT_LIST_DIR}/token.h"
    )
