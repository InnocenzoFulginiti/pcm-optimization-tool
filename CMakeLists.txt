# set required cmake version
cmake_minimum_required(VERSION 3.19)
project(
        qcprop
        LANGUAGES CXX
        DESCRIPTION "qcprop - Quantum Constant Propagation")

# Include qfr
# check whether the submodule ``modulename`` is correctly cloned in the ``/extern`` directory.
macro(CHECK_SUBMODULE_PRESENT modulename)
    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/${modulename}/CMakeLists.txt")
        message(
                FATAL_ERROR
                "${modulename} submodule not cloned properly. \
        Please run `git submodule update --init --recursive` \
        from the main project directory")
    endif ()
endmacro()

check_submodule_present(qfr)

add_subdirectory(src)

# Get the current working branch
execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get the latest commit hash
execute_process(
        COMMAND git rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

add_executable(${PROJECT_NAME}_main src/main.cpp)
target_link_libraries(${PROJECT_NAME}_main PUBLIC ${PROJECT_NAME})

add_library(Catch INTERFACE)
target_include_directories(Catch
        INTERFACE
        test/
        extern/qfr/include
        )

Include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.3.2 # or a later release
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Catch2)
add_executable(tests
        test/main.cpp
        test/UnionTableTest.cpp
        test/ComplexTest.cpp
        test/QubitStateTest.cpp
        test/OperationTest.cpp
        test/ConstantPropagationTest.cpp
        test/TestUtils.cpp
        test/Benchmark.cpp
        test/BitSetTest.cpp)

target_compile_definitions(tests PRIVATE
        "-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")

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
        extern/qfr/include
        )

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain
        ${PROJECT_NAME}
        if (WIN32)
        -static-libgcc -static-libstdc++
        endif (WIN32)
        )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg
    if(WIN32)
        -static
    endif(WIN32)
")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

install(
        TARGETS tests
        RUNTIME DESTINATION bin
)

include(CPack)