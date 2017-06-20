cmake_minimum_required (VERSION 3.1)
project (Hopper)

set (MAIN_TARGET_NAME "gracc")

option(EXPORT_COMPILE_COMMANDS "EXPORT_COMPILE_COMMANDS" ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ${EXPORT_COMPILE_COMMANDS})

# The version number.
set (Hopper_VERSION_MAJOR 0)
set (Hopper_VERSION_MINOR 1)

set (PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set (PROJECT_TEST_SOURCE_DIR ${CMAKE_SOURCE_DIR}/test)

set (EXT_PROJECTS_DIR ${CMAKE_SOURCE_DIR}/ext)

set(PROJECT_TEST_NAME ${MAIN_TARGET_NAME}_test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -Wall \
    -Wextra \
    -Wno-long-long \
    -pedantic \
    -Werror \
    ")



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

include(cmake/Sources.cmake)
include(cmake/Tests.cmake)

set(GRACC_LIBRARY grasshopper)

add_library(${GRACC_LIBRARY} STATIC "")
set_property(TARGET ${GRACC_LIBRARY} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${GRACC_LIBRARY} PROPERTY CXX_STANDARD_REQUIRED ON)
TARGET_LINK_LIBRARIES(${GRACC_LIBRARY}
    PUBLIC
        ${Boost_LIBRARIES}
    )

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(${PROJECT_SOURCE_DIR})

enable_testing()
include_directories(${GTEST_INCLUDE_DIRS} ${COMMON_INCLUDES})

add_subdirectory(${PROJECT_TEST_SOURCE_DIR})

