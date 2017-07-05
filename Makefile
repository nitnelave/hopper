BUILD_FOLDER=build
TARGET=gracc
NINJA=$(shell which ninja 2>&1)
ifneq (${NINJA}, )
  BUILD_COMMAND=${NINJA}
  CMAKE_FLAGS=-GNinja -DCMAKE_MAKE_PROGRAM="${NINJA}"
  BUILD_FILE=${BUILD_FOLDER}/build.ninja
else
  BUILD_COMMAND=make
  CMAKE_FLAGS=""
  BUILD_FILE=${BUILD_FOLDER}/Makefile
endif

CMAKE_FLAGS+="-DCMAKE_BUILD_TYPE=Debug"

all: ${TARGET}

${TARGET}: cmake
	cd ${BUILD_FOLDER} && ${BUILD_COMMAND}

cmake: ${BUILD_FILE}

${BUILD_FILE}: ${BUILD_FOLDER}
	cd ${BUILD_FOLDER} && cmake ${CMAKE_FLAGS} ..

${BUILD_FOLDER}:
	mkdir -p $@

test: cmake
	cd ${BUILD_FOLDER} && ${BUILD_COMMAND} test

check: cmake
	cd ${BUILD_FOLDER} && ${BUILD_COMMAND} check

.PHONY: all ${TARGET} cmake test check
