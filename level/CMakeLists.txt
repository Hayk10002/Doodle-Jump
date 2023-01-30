set(LevelTargetName level)

project(${LevelTargetName})

add_library(${LevelTargetName} STATIC
            src/level/Level.hpp
            src/level/Level.cpp
            src/level/LevelGenerator.hpp
            src/level/LevelGenerator.cpp
)

target_link_libraries(${LevelTargetName}
        PUBLIC
            AllLibraries
            common
            gameObjects
)  

target_include_directories(${LevelTargetName}
        PUBLIC
            src
)