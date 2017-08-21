BUILD_FOLDER=build
TARGET=gracc
NINJA=$(shell which ninja 2>&1)
ifneq (${NINJA}, )
  BUILD_BINARY=${NINJA}
  CMAKE_FLAGS=-GNinja -DCMAKE_MAKE_PROGRAM="${NINJA}"
  BUILD_FILE=build.ninja
else
  BUILD_BINARY=make
  CMAKE_FLAGS=""
  BUILD_FILE=Makefile
endif

.SECONDEXPANSION:

BUILD_COMMAND=${BUILD_BINARY} -j8

CMAKE_FLAGS+="-DCMAKE_BUILD_TYPE=Debug"

all: ${TARGET}

${TARGET}: cmake
	cd ${BUILD_FOLDER} && ${BUILD_COMMAND}

# Dirty hack to propagate the target-dependent value of BUILD_FOLDER
cmake:
	$(MAKE) CMAKE_FLAGS="$(CMAKE_FLAGS)" $(BUILD_FOLDER)/${BUILD_FILE}

%/${BUILD_FILE}: %
	cd $^ && cmake $(CMAKE_FLAGS) ..

${BUILD_FOLDER}:
	mkdir -p $@

build_coverage:
	mkdir -p $@

test: cmake
	cd $(BUILD_FOLDER) && ${BUILD_COMMAND} test

check: cmake
	cd $(BUILD_FOLDER) && ${BUILD_COMMAND} check

clean: cmake
	cd $(BUILD_FOLDER) && ${BUILD_COMMAND} clean

coverage: BUILD_FOLDER=build_coverage
coverage: CMAKE_FLAGS+=-DENABLE_COVERAGE=ON

coverage: check
	cd ${BUILD_FOLDER} && ${BUILD_COMMAND} gracc_coverage

.PHONY: all ${TARGET} cmake test check coverage
