target_sources(${PROJECT_TEST_NAME}
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}/resources_utils.h"
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/resources_utils.cc"
        "${CMAKE_CURRENT_LIST_DIR}/resources.cc"
    )
