add_hopper_library(gracc_lex)

target_sources(gracc_lex
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/lexer.cc"
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/lexer.h"
        "${CMAKE_CURRENT_LIST_DIR}/token.h"
    )
