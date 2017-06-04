#! /bin/sh

#####################################################################
# Install Boost headers
#####################################################################

if [[ "${BOOST_VERSION}" != "" ]]; then
  BOOST_DIR=${DEPS_DIR}/boost-${BOOST_VERSION}
  INSTALL_DIR=${DEPS_DIR}/b2
  if [[ -z "$(ls -A ${BOOST_DIR} 2>/dev/null)" ]]; then
    if [[ "${BOOST_VERSION}" == "trunk" ]]; then
      BOOST_URL="http://github.com/boostorg/boost.git"
      travis_retry git clone --depth 1 --recursive --quiet ${BOOST_URL} ${BOOST_DIR} || exit 1
      (cd ${BOOST_DIR} && ./bootstrap.sh && ./b2 headers)
    else
      BOOST_URL="http://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/boost_${BOOST_VERSION//\./_}.tar.gz"
      mkdir -p ${BOOST_DIR}
      { travis_retry wget --quiet -O - ${BOOST_URL} | tar --strip-components=1 -xz -C ${BOOST_DIR}; } || exit 1
    fi
  fi
  (cd ${BOOST_DIR} \
    && ./bootstrap.sh --prefix=${INSTALL_DIR} \
    && ./b2 install --toolset=clang \
      cxxflags="-stdlib=libc++ -std=c++11 -I${LLVM_ROOT}/install/include/c++/v1 -stdlib=libc++" \
      linkflags="-stdlib=libc++ -L${LLVM_ROOT}/install/lib" \
      -q -d0 --prefix=${INSTALL_DIR} \
      --with-program_options \
      -j${JOBS})
  export PATH=${DEPS_DIR}/b2/bin:${PATH}
fi
