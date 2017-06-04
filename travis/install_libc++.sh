#! /bin/sh


LLVM_VERSION="3.8.1"
LLVM_ROOT="${DEPS_DIR}/llvm-${LLVM_VERSION}"
LLVM_URL="http://llvm.org/releases/${LLVM_VERSION}/llvm-${LLVM_VERSION}.src.tar.xz"
LIBCXX_URL="http://llvm.org/releases/${LLVM_VERSION}/libcxx-${LLVM_VERSION}.src.tar.xz"
LIBCXXABI_URL="http://llvm.org/releases/${LLVM_VERSION}/libcxxabi-${LLVM_VERSION}.src.tar.xz"
CXXFLAGS=""
CC=gcc-5
CXX=g++-5
if [[ -z "$(ls -A ${LLVM_ROOT}/install/include)" ]]; then
  mkdir -p "${LLVM_ROOT}" "${LLVM_ROOT}/build" "${LLVM_ROOT}/projects/libcxx" "${LLVM_ROOT}/projects/libcxxabi"
  travis_retry wget --quiet -O - "${LLVM_URL}" | tar --strip-components=1 -xJ -C "${LLVM_ROOT}"
  travis_retry wget --quiet -O - "${LIBCXX_URL}" | tar --strip-components=1 -xJ -C "${LLVM_ROOT}/projects/libcxx"
  travis_retry wget --quiet -O - "${LIBCXXABI_URL}" | tar --strip-components=1 -xJ -C "${LLVM_ROOT}/projects/libcxxabi"
  (cd "${LLVM_ROOT}/build" && cmake .. -DCMAKE_INSTALL_PREFIX="${LLVM_ROOT}/install" -DCMAKE_BUILD_TYPE=Release)
  (cd "${LLVM_ROOT}/build/projects/libcxx" && make install -j$JOBS)
  (cd "${LLVM_ROOT}/build/projects/libcxxabi" && make install -j$JOBS)
fi
export LLVM_ROOT
export CXXFLAGS="${CXXFLAGS} -I ${LLVM_ROOT}/install/include/c++/v1 -stdlib=libc++"
export LDFLAGS="-L ${LLVM_ROOT}/install/lib -lc++ -lc++abi"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${LLVM_ROOT}/install/lib"
