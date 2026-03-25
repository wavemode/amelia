cmake_minimum_required(VERSION 3.23)

project(amelia)

add_executable(amelia)

target_sources(amelia PRIVATE
    src/main.cpp
)

target_include_directories(amelia PRIVATE src)

target_compile_features(amelia PRIVATE cxx_std_17)

target_compile_options(amelia PRIVATE -Wall -Wextra -Wpedantic)
