find_package(Threads REQUIRED)

add_executable(${PROJECT_TEST_NAME} "main.cc")


include(error/CMakeLists.txt)
include(lexer/CMakeLists.txt)
include(test_utils/CMakeLists.txt)

set_property(TARGET ${PROJECT_TEST_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_TEST_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
target_include_directories(${PROJECT_TEST_NAME} PRIVATE ".")

target_link_libraries(${PROJECT_TEST_NAME}
    ${GTEST_LIBS_DIR}/libgtest.a
    ${GTEST_LIBS_DIR}/libgtest_main.a
    ${GRACC_LIBRARY}
    )

add_dependencies(${PROJECT_TEST_NAME} googletest)

target_link_libraries(${PROJECT_TEST_NAME} ${CMAKE_THREAD_LIBS_INIT})

add_memcheck_test(gtest ${PROJECT_TEST_NAME})
