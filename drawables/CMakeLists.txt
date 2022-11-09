set(DrawablesTargetName drawables)

project(${DrawablesTargetName})

add_library(${DrawablesTargetName} STATIC
            src/drawables/ImageBackground.hpp
            src/drawables/ImageBackground.cpp
)

target_link_libraries(${DrawablesTargetName}
        PUBLIC
            AllLibraries
)  

target_include_directories(${DrawablesTargetName}
        PUBLIC
            src
)