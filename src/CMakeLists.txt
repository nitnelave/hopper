add_executable(${MAIN_TARGET_NAME} main.cc)
include(ast/CMakeLists.txt)
include(codegen/CMakeLists.txt)
include(error/CMakeLists.txt)
include(lexer/CMakeLists.txt)
include(name_resolution/CMakeLists.txt)
include(parser/CMakeLists.txt)
include(pretty_printer/CMakeLists.txt)
include(transform/CMakeLists.txt)
include(util/CMakeLists.txt)
include(visitor/CMakeLists.txt)

set_property(TARGET ${MAIN_TARGET_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${MAIN_TARGET_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

TARGET_LINK_LIBRARIES(${MAIN_TARGET_NAME}
    PUBLIC
        ${GRACC_LIBRARY}
        ${GRACC_LLVM_LIBRARY}
    )

if (ENABLE_COVERAGE)
  target_compile_options(${MAIN_TARGET_NAME} PUBLIC -g -O0 -fprofile-arcs -ftest-coverage)
endif(ENABLE_COVERAGE)
