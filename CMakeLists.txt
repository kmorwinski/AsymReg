project(asymreg)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(asymreg_SRCS asymreg.cpp main.cpp)
qt4_automoc(${asymreg_SRCS})
add_executable(asymreg ${asymreg_SRCS})
target_link_libraries(asymreg ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
install(TARGETS asymreg RUNTIME DESTINATION bin)
