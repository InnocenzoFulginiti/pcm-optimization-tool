cmake_minimum_required(VERSION 3.2)

project(qcprop)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set(QCPROP_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(QCPROP_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(QCPROP_INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

set(QCPROP_HEADERS_DIR ${PROJECT_SOURCE_DIR}/src/circuits)

include_directories(${QCPROP_INSTALL_INCLUDE_DIR})
include_directories(${QCPROP_HEADERS_DIR})

add_subdirectory(src)
#add_subdirectory(test)


