cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)

project(DoodleJump VERSION 0.0.1)

add_subdirectory("C:/Users/Hayk/Programming/AllLibraries" ${CMAKE_BINARY_DIR}/AllLibraries)

add_subdirectory(config)

add_subdirectory(main)
