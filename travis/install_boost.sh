#! /bin/sh

#####################################################################
# Install Boost headers
#####################################################################

if [[ "${BOOST_VERSION}" != "" ]]; then
  BOOST_DIR=${DEPS_DIR}/boost-${BOOST_VERSION}
  if [[ -z "$(ls -A ${BOOST_DIR})" ]]; then
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
  CMAKE_OPTIONS+=" -DBOOST_ROOT=${BOOST_DIR}"
  (cd ${BOOST_DIR}/tools/build && ./bootstrap.sh && ./b2 install --prefix=${DEPS_DIR}/b2)
  (cd ${BOOST_DIR} && ./bootstrap.sh && ./b2 install --prefix=${DEPS_DIR}/b2)
  export PATH=${DEPS_DIR}/b2/bin:${PATH}
fi
