cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)

project(DoodleJump VERSION 0.0.1)

set(ResourceDirectoryPath ${CMAKE_CURRENT_SOURCE_DIR}/resources CACHE PATH "Path to the resource folder")

add_subdirectory("C:/Users/Hayk/Programming/AllLibraries" ${CMAKE_BINARY_DIR}/AllLibraries)

add_subdirectory(config)

add_subdirectory(common)

add_subdirectory(drawables)

add_subdirectory(level)

add_subdirectory(main)
