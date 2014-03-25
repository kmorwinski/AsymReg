project(asymreg)
cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# C++11 Support
include(CXX11)
check_for_cxx11_compiler(CXX11_COMPILER)
if(CXX11_COMPILER)
    enable_cxx11()
else()
    message(FATAL_ERROR "C++11 standard is requiered for this project!")
endif()

# QT 4.8
find_package(Qt4 4.8 REQUIRED)

# Eigen 3.2.1
find_package(Eigen3 3.2.1 REQUIRED)

# DISLIN 10.4.1
set(DISLIN_PATH "/usr/local/dislin")
set(DISLIN_LIB "discpp")

# source files
set(asymreg_SRCS
    main.cpp
    mainwindow.cpp
    interpol.cpp
    plotter.cpp
)

# header files
include_directories(
    ${QT_INCLUDES}
    ${EIGEN3_INCLUDE_DIR}
    ${DISLIN_PATH}
    ${CMAKE_CURRENT_BINARY_DIR}
)

# build & link executable
qt4_automoc(${asymreg_SRCS})

add_executable(asymreg ${asymreg_SRCS})

target_link_libraries(asymreg
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${DISLIN_LIB}
)

# intall
install(TARGETS asymreg RUNTIME DESTINATION bin)
