cmake_minimum_required (VERSION 3.1)
project (Hopper)

set (MAIN_TARGET_NAME "gracc")

option(EXPORT_COMPILE_COMMANDS "EXPORT_COMPILE_COMMANDS" ON)
option(ENABLE_COVERAGE "ENABLE_COVERAGE" OFF)
option(ENABLE_TESTS_WITH_VALGRIND "ENABLE_TESTS_WITH_VALGRIND" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ${EXPORT_COMPILE_COMMANDS})

# The version number.
set(Hopper_VERSION_MAJOR 0)
set(Hopper_VERSION_MINOR 2)
set(Hopper_VERSION_PATCH 0)
set(Hopper_RELEASE_MARKER a)

set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(PROJECT_TEST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/test)
set(PROJECT_TEST_RESOURCE_DIR ${PROJECT_TEST_SOURCE_DIR}/resources)

set(EXT_PROJECTS_DIR ${CMAKE_SOURCE_DIR}/ext)
set(TOOLS_DIR ${CMAKE_SOURCE_DIR}/tools)

set(PROJECT_TEST_NAME ${MAIN_TARGET_NAME}_test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -Wall \
    -Wextra \
    -Wno-long-long \
    -pedantic \
    -Werror \
    -fno-rtti \
    ")

set(TEST_FLAGS "--test_resource_folder=${PROJECT_TEST_RESOURCE_DIR}")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME ON)

find_package(gflags REQUIRED)
find_package(LLVM REQUIRED CONFIG)
find_package(Boost REQUIRED)
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/HopperConfig.h.in"
  "${PROJECT_BINARY_DIR}/HopperConfig.h"
  )

# add the binary tree to the search path for include files
# so that we will find HopperConfig.h
include_directories("${PROJECT_BINARY_DIR}")

# Setup GTest
add_subdirectory(${EXT_PROJECTS_DIR}/gtest)


include(CodeCoverage)
include(Sources)
include(Tests)

set(GRACC_LIBRARY grasshopper)
set(GRACC_LLVM_LIBRARY grasshopper_llvm)

add_library(${GRACC_LIBRARY} STATIC "")
set_property(TARGET ${GRACC_LIBRARY} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${GRACC_LIBRARY} PROPERTY CXX_STANDARD_REQUIRED ON)
TARGET_LINK_LIBRARIES(${GRACC_LIBRARY}
    PUBLIC
        gflags
    )

add_library(${GRACC_LLVM_LIBRARY} STATIC "")
set_property(TARGET ${GRACC_LLVM_LIBRARY} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${GRACC_LLVM_LIBRARY} PROPERTY CXX_STANDARD_REQUIRED ON)
llvm_map_components_to_libnames(llvm_libs x86asmparser x86codegen)
TARGET_LINK_LIBRARIES(${GRACC_LLVM_LIBRARY}
    PUBLIC
        ${GRACC_LIBRARY}
        ${llvm_libs}
        gflags
    )


include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(${PROJECT_SOURCE_DIR})

enable_testing()
include_directories(${GTEST_INCLUDE_DIRS} ${COMMON_INCLUDES})

add_subdirectory(${PROJECT_TEST_SOURCE_DIR})

if (ENABLE_COVERAGE)
  set(GCOV_PATH ${TOOLS_DIR}/llvm-gcov)
  set(LCOV_PATH ${TOOLS_DIR}/lcov.sh)
  set(COVERAGE_EXCLUDES
    "${CMAKE_SOURCE_DIR}\/test/*"
    "${CMAKE_SOURCE_DIR}\/test/*/*"
    "*gtest/*"
    "*gflags/*")
  setup_target_for_coverage(NAME ${MAIN_TARGET_NAME}_coverage EXECUTABLE
    ${PROJECT_TEST_NAME} ${TEST_FLAGS}
    DEPENDENCIES ${PROJECT_TEST_NAME})
endif(ENABLE_COVERAGE)

add_custom_target(check
    ${CMAKE_COMMAND} -E echo CWD=${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E echo CMD=${CMAKE_CTEST_COMMAND} -C $<CONFIG>
    COMMAND ${CMAKE_COMMAND} -E echo ----------------------------------
    COMMAND ${CMAKE_COMMAND} -E env CTEST_OUTPUT_ON_FAILURE=1
            ${CMAKE_CTEST_COMMAND} -C $<CONFIG>
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS ${PROJECT_TEST_NAME}
  )
