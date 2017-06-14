cmake_minimum_required(VERSION 2.8.8)
project(gtest_builder C CXX)
include(ExternalProject)

set(GTEST_FORCE_SHARED_CRT ON)
set(GTEST_DISABLE_PTHREADS OFF)
if (NOT DEFINED GTEST_INSTALL_PATH)
  set(GTEST_INSTALL_PATH "${CMAKE_CURRENT_BINARY_DIR}")
endif()

if(MINGW)
    set(GTEST_DISABLE_PTHREADS ON)
endif()

set(GTEST_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11")

configure_file(wrapper.sh.in wrapper.sh @ONLY)
ExternalProject_Add(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    CMAKE_ARGS
    -Dgtest_force_shared_crt=${GTEST_FORCE_SHARED_CRT}
    -Dgtest_disable_pthreads=${GTEST_DISABLE_PTHREADS}
    -DBUILD_GTEST=ON
    PREFIX "${GTEST_INSTALL_PATH}"
    # Disable install step
    INSTALL_COMMAND ""
    CONFIGURE_COMMAND mkdir -p ${GTEST_INSTALL_PATH}/src/googletest-build && cd ${GTEST_INSTALL_PATH}/src/googletest-build && ${CMAKE_CURRENT_BINARY_DIR}/wrapper.sh cmake ... ${GTEST_INSTALL_PATH}/src/googletest
    BUILD_COMMAND /bin/sh ${CMAKE_CURRENT_BINARY_DIR}/wrapper.sh $(MAKE)
    )

# Specify include dir
ExternalProject_Get_Property(googletest source_dir)
set(GTEST_INCLUDE_DIRS ${source_dir}/googletest/include PARENT_SCOPE)

# Specify MainTest's link libraries
ExternalProject_Get_Property(googletest binary_dir)
set(GTEST_LIBS_DIR ${binary_dir}/googlemock/gtest PARENT_SCOPE)
