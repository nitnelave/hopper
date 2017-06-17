add_executable(${MAIN_TARGET_NAME} main.cc)
include(lexer/CMakeLists.txt)
include(parser/CMakeLists.txt)

set_property(TARGET ${MAIN_TARGET_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${MAIN_TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
