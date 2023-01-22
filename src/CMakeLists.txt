cmake_minimum_required(VERSION 3.2)
project(qcprop)

add_subdirectory(circuit)
add_subdirectory(parser)

set(SOURCE_FILES main.cpp)

add_executable(qcprop ${SOURCE_FILES})
target_link_libraries(qcprop circuit parser)
install(TARGETS qcprop DESTINATION ${QCPROP_INSTALL_BIN_DIR})
