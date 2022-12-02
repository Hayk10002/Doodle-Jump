set(LevelTargetName level)

project(${LevelTargetName})

add_library(${LevelTargetName} STATIC
            src/level/Level.hpp
            src/level/Level.cpp
)

target_link_libraries(${LevelTargetName}
        PUBLIC
            AllLibraries
            common
            drawables
)  

target_include_directories(${LevelTargetName}
        PUBLIC
            src
)