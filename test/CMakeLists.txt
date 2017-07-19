find_package(Threads REQUIRED)

add_executable(${PROJECT_TEST_NAME} "main.cc")


include(codegen/CMakeLists.txt)
include(error/CMakeLists.txt)
include(lexer/CMakeLists.txt)
include(parser/CMakeLists.txt)
include(resources/CMakeLists.txt)
include(test_utils/CMakeLists.txt)
include(visitor/CMakeLists.txt)

set_property(TARGET ${PROJECT_TEST_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_TEST_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
target_include_directories(${PROJECT_TEST_NAME} PRIVATE ".")

target_link_libraries(${PROJECT_TEST_NAME}
    PUBLIC
    ${GTEST_LIBS_DIR}/libgtest.a
    ${GTEST_LIBS_DIR}/libgtest_main.a
    ${GRACC_LIBRARY}
    ${GRACC_LLVM_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    )

add_dependencies(${PROJECT_TEST_NAME} googletest)

add_memcheck_test(gtest ${PROJECT_TEST_NAME})

if (ENABLE_COVERAGE)
  target_compile_options(${GRACC_LIBRARY} PUBLIC -g -O0 -fprofile-arcs -ftest-coverage)
  target_link_libraries(${PROJECT_TEST_NAME} PUBLIC "-fprofile-arcs -ftest-coverage -lgcov --coverage")
endif(ENABLE_COVERAGE)
