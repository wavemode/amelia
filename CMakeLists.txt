cmake_minimum_required(VERSION 3.23)

project(amelia)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Enabling Sanitizers: Address and Undefined Behavior")
    add_compile_options("-fsanitize=address,undefined" "-fno-omit-frame-pointer" "-g1")
    add_link_options("-fsanitize=address,undefined")
endif()

file(GLOB_RECURSE SOURCES_SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/src/system/*.cpp")
file(GLOB_RECURSE SOURCES_ACTION "${CMAKE_CURRENT_SOURCE_DIR}/src/action/*.cpp")
file(GLOB_RECURSE SOURCES_UTIL "${CMAKE_CURRENT_SOURCE_DIR}/src/util/*.cpp")
file(GLOB_RECURSE SOURCES_DATA "${CMAKE_CURRENT_SOURCE_DIR}/src/data/*.cpp")
file(GLOB_RECURSE SOURCES_EFFECT "${CMAKE_CURRENT_SOURCE_DIR}/src/effect/*.cpp")
file(GLOB_RECURSE SOURCES_TEST "${CMAKE_CURRENT_SOURCE_DIR}/src/test/*.cpp")

add_library(amelia_warnings INTERFACE)
target_compile_options(
    amelia_warnings INTERFACE
    -Wall -Wextra -Wpedantic -Werror
    -Wno-error=unused-parameter
    -Wno-error=unused-variable
    -Wno-error=unused-function
)

add_library(amelia_core
    ${SOURCES_SYSTEM}
    ${SOURCES_ACTION}
    ${SOURCES_UTIL}
    ${SOURCES_DATA}
    ${SOURCES_EFFECT}
)
target_include_directories(amelia_core PUBLIC src vendor)
target_compile_features(amelia_core PUBLIC cxx_std_20)
target_link_libraries(amelia_core PRIVATE amelia_warnings)

add_executable(amelia
    src/main.cpp
)
target_link_libraries(amelia PRIVATE amelia_core)
target_link_libraries(amelia PRIVATE amelia_warnings)

add_executable(amelia_test
    ${SOURCES_TEST}
)
target_link_libraries(amelia_test PRIVATE amelia_core)
