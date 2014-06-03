# CMake Project
project(asymreg)
cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(asymreg_BIN "${PROJECT_NAME}-schlieren")
set(asymreg_cl_BIN "${asymreg_BIN}-cl")

#check if CMAKE_BUILD_TYPE is valid
include(CheckBuildType)
check_build_type()

# C++11 Support
include(CXX11)
check_for_cxx11_compiler(CXX11_COMPILER)
if(CXX11_COMPILER)
    enable_cxx11()
else()
    message(FATAL_ERROR "C++11 standard is requiered for this project!")
endif()

# QT 4.8
find_package(Qt4 4.8 COMPONENTS QtSvg REQUIRED)
find_package(QtIOCompressor)
if(QTIOCOMPRESSOR_FOUND)
    add_definitions(-DQT_HAS_IOCOMPRESSOR)
endif()
find_package(QJSON 0.8.1 REQUIRED)
string(TOUPPER ${CMAKE_BUILD_TYPE} build_type)
if(NOT ${build_type} STREQUAL DEBUG)
    add_definitions(-DQT_NO_DEBUG) # skip all Q_ASSERT's
endif()

# Eigen 3.2.1
find_package(Eigen3 3.2.1 REQUIRED)
add_definitions(-DEIGEN_DENSEBASE_ADDONS_FILE=\"${PROJECT_SOURCE_DIR}/eigen_densebaseaddons.h\")

# DISLIN 10.4.1
set(DISLIN_PATH "/usr/local/dislin")
set(DISLIN_LIB "discpp")

# source files
set(asymreg_COMMON_SRCS
    asymreg.cpp
    duration.cpp
    interpol.cpp
)

set(asymreg_COMMON_HDRS
    backprojection.h
    eigen.h
    eigen_addons.h
    eigen_iterator.h
    eigen_io.h
    radonoperator.h
)

set(asymreg_GUI_SRCS
    mainwindow.cpp
    plotter.cpp
    plottersettings.cpp
    plottersettingsdialog.cpp
    svgviewer.cpp
)

set(asymreg_GUI_HDRS "")

# set proper include dirs:
include_directories(
    ${QT_INCLUDES}
    ${QTIOCOMPRESSOR_INCLUDE_DIR}
    ${QJSON_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${DISLIN_PATH}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# build & link executables:
qt4_automoc(${asymreg_GUI_SRCS} ${asymreg_GUI_HDRS})

add_executable(${asymreg_BIN}
    ${asymreg_COMMON_SRCS}
    ${asymreg_COMMON_HDRS}
    ${asymreg_GUI_SRCS}
    ${asymreg_GUI_HDRS}
    main.cpp
)

add_executable(${asymreg_cl_BIN}
    ${asymreg_COMMON_SRCS}
    ${asymreg_COMMON_HDRS}
    main-cl.cpp
)

target_link_libraries(${asymreg_BIN}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTSVG_LIBRARY}
    ${QTIOCOMPRESSOR_LIBRARY}
    ${QJSON_LIBRARIES}
    ${DISLIN_LIB}
)

# intall
install(TARGETS ${asymreg_BIN} ${asymreg_cl_BIN}
    RUNTIME DESTINATION bin
)

# dummy project for additional files to show up in project tree
file(GLOB DATA_FILES
    "${PROJECT_SOURCE_DIR}/data/*.csv"
    "${PROJECT_SOURCE_DIR}/data/*.json"
)
add_custom_target(${PROJECT_NAME}-dummy SOURCES ${DATA_FILES})
