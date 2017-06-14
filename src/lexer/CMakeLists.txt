target_sources(${MAIN_TARGET_NAME}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/lexer.cc"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/lexer.h"
        "${CMAKE_CURRENT_LIST_DIR}/token.h"
    )
