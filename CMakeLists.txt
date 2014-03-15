project(asymreg)
cmake_minimum_required(VERSION 2.8)

# QT 4.8
find_package(Qt4 REQUIRED)

# source files
set(asymreg_SRCS
        main.cpp
        mainwindow.cpp
    )

# header files
include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

# build & link executable
qt4_automoc(${asymreg_SRCS})
add_executable(asymreg ${asymreg_SRCS})
target_link_libraries(asymreg ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

# intall
install(TARGETS asymreg RUNTIME DESTINATION bin)
