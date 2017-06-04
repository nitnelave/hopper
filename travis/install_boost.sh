#! /bin/sh

#####################################################################
# Install Boost headers
#####################################################################

# Verbose
set -x

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
  (cd ${BOOST_DIR} && ./bootstrap.sh --prefix=${INSTALL_DIR} && ./b2 install -q -d0 --prefix=${INSTALL_DIR} --with-program_options -j${JOBS})
  CMAKE_OPTIONS+=" -DCMAKE_INCLUDE_PATH=${INSTALL_DIR} -DCMAKE_LIBRARY_PATH=${INSTALL_DIR}/lib -DBOOST_ROOT=${INSTALL_DIR}"
  export PATH=${DEPS_DIR}/b2/bin:${PATH}
fi
