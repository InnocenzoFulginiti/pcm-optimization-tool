# set required cmake version
cmake_minimum_required(VERSION 3.19)
project(
        qcprop
        LANGUAGES CXX
        DESCRIPTION "qcprop - Quantum Constant Propagation")

# add main library code
#All .cpp and .hpp files in the src folder and include folder
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp" "include/*.hpp")

#Print all found files (maybe remove later)
string (REPLACE ";" "\n" ALL_SOURCES_NL "${ALL_SOURCES}")
message(STATUS "All sources: \n${ALL_SOURCES_NL}")

add_executable("${PROJECT_NAME}" ${ALL_SOURCES})
target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_17)