project(dislin-test)

cmake_minimum_required(VERSION 2.8)

# source files
set(dislin-test_SRCS main.cpp)

# Qt 4
find_package(Qt4 REQUIRED)

# DISLIN 10.4
set(DISLIN_PATH "/usr/local/dislin")
set(DISLIN_LIB "discpp")

# include dirs for header files
include_directories(${DISLIN_PATH} ${QT_INCLUDES})

# lib dirs
link_directories(${DISLIN_PATH})

# build & link executable
add_executable(dislin-test ${dislin-test_SRCS})

target_link_libraries(dislin-test ${DISLIN_LIB} ${QT_QTCORE_LIBRARY})

# intall
install(TARGETS dislin-test RUNTIME DESTINATION bin)
