project(eigen-serializer)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../../cmake)

find_package(Eigen3 3.2.1 REQUIRED)

aux_source_directory(. SRC_LIST)

include_directories(${EIGEN3_INCLUDE_DIR})


file(GLOB DATA_FILES
    "${PROJECT_BINARY_DIR}/data*.txt"
    "${PROJECT_BINARY_DIR}/data*.csv"
)

add_executable(${PROJECT_NAME}
    ${SRC_LIST}
    ${PROJECT_SOURCE_DIR}/../../eigen.h
)

add_custom_target(${PROJECT_NAME}-add-project-files SOURCES ${DATA_FILES})
