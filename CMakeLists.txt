# set required cmake version
cmake_minimum_required(VERSION 3.19)
project(
        qcprop
        LANGUAGES CXX
        DESCRIPTION "qcprop - Quantum Constant Propagation")

# Add local project module path so CMake includes custom CMake modules.
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")

include(CTest)
enable_testing()

# add main library code
#All .cpp and .hpp files in the src folder and include folder
file(GLOB_RECURSE ALL_SOURCES "src/*.cpp" "include/*.hpp")

add_library(qcpropLib
        ${ALL_SOURCES}
        )

target_include_directories(qcpropLib
        PUBLIC
        include
        include/circuit
        include/operations
        include/parser
        include/util
        )

target_compile_options(qcpropLib
        PRIVATE
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Werror -Wall -Wextra>
        $<$<CXX_COMPILER_ID:MSVC>:
        /W4>
        )

#Print all found files (maybe remove later)
string(REPLACE ";" "\n" ALL_SOURCES_NL "${ALL_SOURCES}")
message(STATUS "All sources: \n${ALL_SOURCES_NL}")

add_executable("${PROJECT_NAME}" ${ALL_SOURCES})
target_compile_features("${PROJECT_NAME}" PRIVATE cxx_std_17)

add_library(Catch INTERFACE)
target_include_directories(Catch
        INTERFACE
        test/
        )

Include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.0.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests
        test/main.cpp
        test/UnionTableTest.cpp
        test/ComplexTest.cpp test/QubitStateTest.cpp test/OperationTest.cpp test/ConstantPropagationTest.cpp)


list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)




target_include_directories(tests
        PUBLIC
        test/
        include
        include/circuit
        include/operations
        include/parser
        include/util
        )

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain
                    qcpropLib)

install(
        TARGETS tests
        RUNTIME DESTINATION bin
)

include(CPack)