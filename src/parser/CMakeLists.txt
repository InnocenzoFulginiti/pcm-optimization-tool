cmake_minimum_required(VERSION 3.2)
project(qcprop C CXX)

set(SOURCE_FILES
        Parser.hpp
        Parser.cpp
        QasmParser.hpp
        QasmParser.cpp
        )

add_library(parser SHARED STATIC ${SOURCE_FILES})

install(TARGETS parser DESTINATION ${QCPROP_INSTALL_LIB_DIR})
install(FILES Parser.hpp DESTINATION ${QCPROP_INSTALL_INCLUDE_DIR})
