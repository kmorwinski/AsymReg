project(asymreg)
cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(asymreg_BIN "${PROJECT_NAME}-schlieren")
set(asymreg_cl_BIN "${PROJECT_NAME}-cl")

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

# Eigen 3.2.1
find_package(Eigen3 3.2.1 REQUIRED)

# DISLIN 10.4.1
set(DISLIN_PATH "/usr/local/dislin")
set(DISLIN_LIB "discpp")

# source files
set(asymreg_SRCS
    asymreg.cpp
    main.cpp
    mainwindow.cpp
    interpol.cpp
    plotter.cpp
    plottersettings.cpp
    plottersettingsdialog.cpp
    svgviewer.cpp
)

# header files (w/o equivalent source files)
set(asymreg_SRCS ${asymreg_SRCS}
    eigen.h
)

# commandline-client source files:
set(asymreg_cl_SRCS
    asymreg.cpp
    main-cl.cpp
    #mainwindow.cpp
    interpol.cpp
    #plotter.cpp
    #plottersettings.cpp
    #plottersettingsdialog.cpp
    #svgviewer.cpp
    eigen.h
)

# header files
include_directories(
    ${QT_INCLUDES}
    ${QTIOCOMPRESSOR_INCLUDE_DIR}
    ${QJSON_INCLUDE_DIR}
    ${EIGEN3_INCLUDE_DIR}
    ${DISLIN_PATH}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# build & link executable
qt4_automoc(${asymreg_SRCS})

add_executable(${asymreg_BIN} ${asymreg_SRCS})

target_link_libraries(${asymreg_BIN}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTSVG_LIBRARY}
    ${QTIOCOMPRESSOR_LIBRARY}
    ${QJSON_LIBRARIES}
    ${DISLIN_LIB}
)

add_executable(${asymreg_cl_BIN} ${asymreg_cl_SRCS})

target_link_libraries(${asymreg_cl_BIN}
    #${QT_QTCORE_LIBRARY}
    #${QT_QTGUI_LIBRARY}
    #${QT_QTSVG_LIBRARY}
    #${QTIOCOMPRESSOR_LIBRARY}
    #${QJSON_LIBRARIES}
    #${DISLIN_LIB}
)

# intall
install(TARGETS ${asymreg_BIN} RUNTIME DESTINATION bin)

# dummy project for additional files to show up in project tree
file(GLOB DATA_FILES
    "${PROJECT_SOURCE_DIR}/data/*.csv"
    "${PROJECT_SOURCE_DIR}/data/*.json"
)
add_custom_target(${PROJECT_NAME}-dummy SOURCES ${DATA_FILES})
