project(asymreg)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

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
