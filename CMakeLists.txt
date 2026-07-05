cmake_minimum_required(VERSION 3.29)

project(amelia)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_C_FLAGS_DEBUG "-O0")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Enabling Sanitizers: Address and Undefined Behavior")
    add_compile_options("-fsanitize=address,undefined" "-fno-omit-frame-pointer" "-g" "-gsplit-dwarf")
    add_link_options("-fsanitize=address,undefined" "-gsplit-dwarf")
endif()

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE SOURCES_TEST "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
set(SOURCES_VENDOR
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/sha2.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/xxhash.c"
)

add_library(amelia_warnings INTERFACE)
target_compile_options(
    amelia_warnings INTERFACE
    -Wall -Wextra -Wpedantic -Werror -Wimplicit-fallthrough
    -Wno-error=unused-parameter
    -Wno-error=unused-variable
    -Wno-error=unused-function
    -Wno-error=unused-but-set-variable
)

add_library(amelia_core
    ${SOURCES}
    ${SOURCES_VENDOR}
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
