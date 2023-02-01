set(GameObjectsTargetName gameObjects)

project(${GameObjectsTargetName})

add_library(${GameObjectsTargetName} STATIC 
            src/gameObjects/Doodle.hpp
            src/gameObjects/Doodle.cpp
            src/gameObjects/Tiles.hpp
            src/gameObjects/Tiles.cpp
            src/gameObjects/Items.hpp
            src/gameObjects/Items.cpp
)

target_link_libraries(${GameObjectsTargetName}
        PUBLIC
            AllLibraries
            common
)  

target_include_directories(${GameObjectsTargetName}
        PUBLIC
            src
)