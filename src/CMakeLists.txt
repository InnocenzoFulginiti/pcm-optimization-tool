# add qfr Library
add_subdirectory("${PROJECT_SOURCE_DIR}/extern/qfr" "extern/qfr" EXCLUDE_FROM_ALL)

file(GLOB_RECURSE SOURCES "./*.cpp" "../include/*.hpp")

add_library(
        ${PROJECT_NAME}
        ${SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}/include)

target_link_libraries(${PROJECT_NAME} PUBLIC project_options project_warnings)

set(CMAKE_CXX_CLANG_TIDY
        clang-tidy;
        -header-filter=.;
        -checks=*;
        -warnings-as-errors=*;)

target_link_libraries(${PROJECT_NAME} PUBLIC qfr)