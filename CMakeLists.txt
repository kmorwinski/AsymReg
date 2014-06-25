# CMake Project
project(asymreg)
cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # include build dir
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
#set(CMAKE_VERBOSE_MAKEFILE 1) # verbose compiler output

# set some project variables
set(asymreg_BIN "${PROJECT_NAME}-schlieren")
set(asymreg_cl_BIN "${asymreg_BIN}-cl")
string(TOUPPER "${CMAKE_BUILD_TYPE}" asymreg_BUILD_TYPE)
set(asymreg_OPTI "-O3") #uncomment to overwrite CMAKE_BUILD_TYPE's default compiler optimisation level

# Compiler Flags
include(CheckBuildType)
check_build_type() #check if CMAKE_BUILD_TYPE is valid

if(CMAKE_COMPILER_IS_GNUCXX AND (NOT "${asymreg_OPTI}" STREQUAL ""))
    # set compiler optimisation level to asymreg_OPTI (if defined)
    string(REGEX REPLACE
           "-O[0123s]" "${asymreg_OPTI}"
           asymreg_CXX_FLAGS
           ${CMAKE_CXX_FLAGS_${asymreg_BUILD_TYPE}}
    )
    set(CMAKE_CXX_FLAGS_${asymreg_BUILD_TYPE} "${asymreg_CXX_FLAGS}")
endif()

# C++11 Support
include(CXX11)
check_for_cxx11_compiler(CXX11_COMPILER)
if(CXX11_COMPILER)
    enable_cxx11()
else()
    message(FATAL_ERROR "C++11 standard is requiered for this project!")
endif()

# OpenMp
find_package(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# QT 4.8
find_package(Qt4 4.8 COMPONENTS QtCore QtGui QtSvg REQUIRED)
include(${QT_USE_FILE}) # sets -DQT_NO_DEBUG and other useful things

find_package(QtIOCompressor)
if(QTIOCOMPRESSOR_FOUND)
    add_definitions(-DQT_HAS_IOCOMPRESSOR)
endif()

find_package(QJSON 0.8.1 REQUIRED)

# Eigen 3.2.1
find_package(Eigen3 3.2.1 REQUIRED)
add_definitions(-DEIGEN_DENSEBASE_ADDONS_FILE=\"${PROJECT_SOURCE_DIR}/eigen_densebaseaddons.h\")

# DISLIN 10.4.1
set(DISLIN_PATH "/usr/local/dislin")
set(DISLIN_LIB "discpp")

# 3rdparty sources & libraries
add_subdirectory(3rdparty) # this will set 3rdparty_SRCS & 3rdparty_HDRS

# source files
set(asymreg_COMMON_SRCS
    asymreg.cpp
    duration.cpp
    interpol.cpp
    plotter.cpp
    plottersettings.cpp
)

set(asymreg_COMMON_HDRS
    backprojection.h
    constants.h
    eigen.h
    eigen_addons.h
    eigen_iterator.h
    eigen_io.h
    ode.h
    radonoperator.h
)

set(asymreg_GUI_SRCS
    datasourcetablewidget.cpp
    mainwindow.cpp
    plottersettingsdialog.cpp
    svgviewer.cpp
)

set(asymreg_GUI_HDRS "")

# set proper include dirs
include_directories(
    ${QTIOCOMPRESSOR_INCLUDE_DIR}
    ${QJSON_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${DISLIN_PATH}
)

# build & link executables
add_executable(${asymreg_BIN} # gui target
    ${asymreg_COMMON_SRCS}
    ${asymreg_COMMON_HDRS}
    ${asymreg_GUI_SRCS}
    ${asymreg_GUI_HDRS}
    main.cpp
)

add_executable(${asymreg_cl_BIN} # cl target
    ${asymreg_COMMON_SRCS}
    ${asymreg_COMMON_HDRS}
    main-cl.cpp
)

target_link_libraries(${asymreg_BIN}
    ${QT_LIBRARIES}
    ${QTIOCOMPRESSOR_LIBRARY}
    ${QJSON_LIBRARIES}
    ${DISLIN_LIB}
    kiconutils                # this will find libkiconutils.so in build-dir automatically
)

target_link_libraries(${asymreg_cl_BIN}
    ${DISLIN_LIB}
)

add_dependencies(${asymreg_BIN} kiconutils) # libkiconutils.so needs to be build for gui target

set_target_properties(${asymreg_BIN} PROPERTIES
    AUTOMOC true # enable MOC on GUI target
)

# Remove Qt4 definitions from cl-build. Otherwise it will fail!!!
get_property(asymreg_DEFS DIRECTORY PROPERTY COMPILE_DEFINITIONS) # get all current definitions
set_property(DIRECTORY PROPERTY COMPILE_DEFINITIONS )             # and unset them (DIRECTORY=global)

set(asymreg_cl_DEFS)
foreach(itr ${asymreg_DEFS})
    if(NOT itr MATCHES "QT_.*") # regexp matches all -DQT_CORE_LIB, -DQT_GUI_LIB, etc.
        list(APPEND asymreg_cl_DEFS ${itr}) # add to list, if it didn't match
    endif()
endforeach()

set_target_properties(${asymreg_BIN} PROPERTIES
    COMPILE_DEFINITIONS "${asymreg_DEFS}" # set them again for gui build
)
set_target_properties(${asymreg_cl_BIN} PROPERTIES
    COMPILE_DEFINITIONS "${asymreg_cl_DEFS}" # build cl without Qt4
)

# dummy project for additional files to show up in project tree
file(GLOB DATA_FILES
    "${PROJECT_SOURCE_DIR}/data/*.csv"
    "${PROJECT_SOURCE_DIR}/data/*.json"
)

add_custom_target(${PROJECT_NAME}-dummy SOURCES ${DATA_FILES})
