# - Try to find the QTIOCOMPRESSOR library
# Once done this will define
#
#  QTIOCOMPRESSOR_FOUND - system has the QTIOCOMPRESSOR library
#  QTIOCOMPRESSOR_INCLUDE_DIR - the QTIOCOMPRESSOR include directory
#  QTIOCOMPRESSOR_LIBRARY - Link this to use the QTIOCOMPRESSOR 
#  QTIOCOMPRESSOR_DEFINITIONS - Compiler switches required for using QtIOCompressor

if (QTIOCOMPRESSOR_INCLUDE_DIR AND QTIOCOMPRESSOR_LIBRARY)
  # in cache already
  set(QTIOCOMPRESSOR_FOUND TRUE)
else (QTIOCOMPRESSOR_INCLUDE_DIR AND QTIOCOMPRESSOR_LIBRARY)
  if (NOT WIN32)
    find_package(PkgConfig)
    pkg_check_modules(PC_QTIOCOMPRESSOR QUIET qtiocompressor)
    set(QTIOCOMPRESSOR_DEFINITIONS ${PC_QTIOCOMPRESSOR_CFLAGS_OTHER})
  endif(NOT WIN32)

  set(PC_QTIOCOMPRESSOR_LIBRARY_DIRS ${PC_QTIOCOMPRESSOR_LIBRARY_DIRS}
    /usr/lib/qtiocompressor
    /usr/lib/QtSolutions
    /usr/lib/
    /usr/lib/x86_64-linux-gnu/qtiocompressor
    /usr/lib/x86_64-linux-gnu/
    /usr/lib/i386-linux-gnu/
    /usr/lib64/QtSolutions
    /usr/lib64/qt4
    /usr/lib64
    /usr/local/lib/qtiocompressor
    /usr/local/lib
    /opt/local/lib/qtiocompressor
    /opt/local/lib/
  )
  set(PC_QTIOCOMPRESSOR_INCLUDE_DIRS ${PC_QTIOCOMPRESSOR_INCLUDE_DIRS}
    qt-solutions/qtiocompressor/src
    qtiocompressor/src
    build/qt-solutions/qtiocompressor/src
    build/qtiocompressor
    /usr/local/include/qtiocompressor
    /usr/local/include
    /usr/include/qtiocompressor
    /usr/include
    /opt/local/include/qtiocompressor
    /opt/local/include
  )

  find_library(QTIOCOMPRESSOR_LIBRARY NAMES QtSolutions_IOCompressor-head QtSolutions_IOCompressor QtSolutions_IOCompressor-2.3 qtiocompressor QtIOCompressor
    HINTS ${PC_QTIOCOMPRESSOR_LIBRARY_DIR} ${PC_QTIOCOMPRESSOR_LIBRARY_DIRS}
    PATH_SUFFIXES lib64/ lib/ qtiocompressor/lib64 qtiocompressor/lib QtSolutions/ qt-solutions/qtiocompressor/lib64 qt-solutions/qtiocompressor/lib
    PATHS
    ${CMAKE_CURRENT_BINARY_DIR}/qt-solutions/
    ${CMAKE_CURRENT_BINARY_DIR}/qtiocompressor/
    ${CMAKE_CURRENT_BINARY_DIR}/lib/
    ${CMAKE_CURRENT_BINARY_DIR}../qt-solutions/
    ${CMAKE_CURRENT_BINARY_DIR}../qtiocompressor/
    ${CMAKE_SOURCE_DIR}/qt-solutions/
    ${CMAKE_SOURCE_DIR}/qtiocompressor/
    ${CMAKE_SOURCE_DIR}/../qt-solutions/
    ${CMAKE_SOURCE_DIR}/../qtiocompressor/
    /usr/lib/qtiocompressor
    /usr/lib/
    /usr/lib64/QtSolutions/
    /usr/lib64/
    /usr/lib/x86_64-linux-gnu/qtiocompressor
    /usr/lib/x86_64-linux-gnu/
    /usr/lib/i386-linux-gnu/
    /usr/local/lib/qtiocompressor
    /usr/local/lib
    /opt/local/lib/qtiocompressor
    /opt/local/lib/
    NO_DEFAULT_PATH
  )
  find_path(QTIOCOMPRESSOR_INCLUDE_DIR qtiocompressor.h
    HINTS ${PC_QTIOCOMPRESSOR_INCLUDEDIR} ${PC_QTIOCOMPRESSOR_INCLUDE_DIRS}
    PATH_SUFFIXES src/ qtiocompressor/src qt-solutions/qtiocompressor/src QtSolutions/
    PATHS
    ${CMAKE_CURRENT_BINARY_DIR}/qt-solutions/
    ${CMAKE_CURRENT_BINARY_DIR}/qtiocompressor/
    ${CMAKE_CURRENT_BINARY_DIR}/lib/
    ${CMAKE_CURRENT_BINARY_DIR}../qt-solutions/
    ${CMAKE_CURRENT_BINARY_DIR}../qtiocompressor/
    ${CMAKE_SOURCE_DIR}/qt-solutions/
    ${CMAKE_SOURCE_DIR}/qtiocompressor/
    ${CMAKE_SOURCE_DIR}/../qt-solutions/
    ${CMAKE_SOURCE_DIR}/../qtiocompressor/
    /usr/local/include/qtiocompressor
    /usr/local/include
    /usr/include/qtiocompressor
    /usr/include
    /opt/local/include/qtiocompressor
    /opt/local/include
    NO_DEFAULT_PATH
  )

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(QtIOCompressor DEFAULT_MSG QTIOCOMPRESSOR_LIBRARY QTIOCOMPRESSOR_INCLUDE_DIR)

  mark_as_advanced(QTIOCOMPRESSOR_INCLUDE_DIR QTIOCOMPRESSOR_LIBRARY)
endif (QTIOCOMPRESSOR_INCLUDE_DIR AND QTIOCOMPRESSOR_LIBRARY)
