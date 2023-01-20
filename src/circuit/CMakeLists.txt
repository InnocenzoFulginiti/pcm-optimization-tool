cmake_minimum_required(VERSION 3.2)
project(qcprop C CXX)

set(SOURCE_FILES
        Circuit.hpp
        Circuit.cpp
        )

add_library(circuit SHARED STATIC ${SOURCE_FILES})

install(TARGETS circuit DESTINATION ${QCPROP_INSTALL_LIB_DIR})
install(FILES Circuit.hpp DESTINATION ${QCPROP_INSTALL_INCLUDE_DIR})
