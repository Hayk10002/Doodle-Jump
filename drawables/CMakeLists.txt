set(DrawablesTargetName drawables)

project(${DrawablesTargetName})

add_library(${DrawablesTargetName} STATIC
            src/drawables/ImageBackground.hpp
            src/drawables/ImageBackground.cpp
            src/drawables/Scene.hpp
            src/drawables/Scene.cpp
)

target_link_libraries(${DrawablesTargetName}
        PUBLIC
            AllLibraries
            common
)  

target_include_directories(${DrawablesTargetName}
        PUBLIC
            src
)