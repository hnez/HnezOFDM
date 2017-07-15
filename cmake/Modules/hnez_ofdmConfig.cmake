INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_HNEZ_OFDM hnez_ofdm)

FIND_PATH(
    HNEZ_OFDM_INCLUDE_DIRS
    NAMES hnez_ofdm/api.h
    HINTS $ENV{HNEZ_OFDM_DIR}/include
        ${PC_HNEZ_OFDM_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    HNEZ_OFDM_LIBRARIES
    NAMES gnuradio-hnez_ofdm
    HINTS $ENV{HNEZ_OFDM_DIR}/lib
        ${PC_HNEZ_OFDM_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HNEZ_OFDM DEFAULT_MSG HNEZ_OFDM_LIBRARIES HNEZ_OFDM_INCLUDE_DIRS)
MARK_AS_ADVANCED(HNEZ_OFDM_LIBRARIES HNEZ_OFDM_INCLUDE_DIRS)

