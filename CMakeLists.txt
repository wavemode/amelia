cmake_minimum_required(VERSION 3.23)

project(amelia)

file(GLOB_RECURSE SOURCES_UNIT "${CMAKE_CURRENT_SOURCE_DIR}/src/unit/*.cpp")
file(GLOB_RECURSE SOURCES_UTIL "${CMAKE_CURRENT_SOURCE_DIR}/src/util/*.cpp")
file(GLOB_RECURSE SOURCES_DATA "${CMAKE_CURRENT_SOURCE_DIR}/src/data/*.cpp")
file(GLOB_RECURSE SOURCES_EFFECT "${CMAKE_CURRENT_SOURCE_DIR}/src/effect/*.cpp")
file(GLOB_RECURSE SOURCES_TEST "${CMAKE_CURRENT_SOURCE_DIR}/src/test/*.cpp")

add_library(amelia_core
    ${SOURCES_UNIT}
    ${SOURCES_UTIL}
    ${SOURCES_DATA}
    ${SOURCES_EFFECT}
)
target_include_directories(amelia_core PUBLIC src)
target_compile_features(amelia_core PUBLIC cxx_std_17)
target_compile_options(amelia_core PUBLIC -Wall -Wextra -Wpedantic)

add_executable(amelia
    src/main.cpp
)
target_link_libraries(amelia PRIVATE amelia_core)

add_executable(amelia_test
    ${SOURCES_TEST}
)
target_link_libraries(amelia_test PRIVATE amelia_core)
