cmake_minimum_required(VERSION 3.24)
project(turing VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

include_directories(SYSTEM ../euler/include)

add_library(pch pch.cpp)
target_precompile_headers(pch PRIVATE pch.hpp)
target_compile_options(pch PRIVATE -fpch-instantiate-templates)
enable_testing()

add_subdirectory(decide)
add_subdirectory(specific)
add_subdirectory(test)

set(targets
    analyze
    enumerate
    tape_growth
    tape_size
    transcript)

foreach(target ${targets})
    message("Adding target: ${target}")
    add_executable(${target} "${target}.cpp")
    target_precompile_headers(${target} REUSE_FROM pch)
endforeach()
