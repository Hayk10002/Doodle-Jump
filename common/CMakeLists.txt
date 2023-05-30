set(CommonTargetName common)

project(${CommonTargetName})

add_library(${CommonTargetName} STATIC
            src/common/Resources.hpp
            src/common/Resources.cpp
            src/common/DebugImGui.hpp
            src/common/DebugImGui.cpp
            src/common/Utils.hpp
            src/common/Utils.cpp
            src/common/GameStuff.hpp
            src/common/GameStuff.cpp
            src/common/Returners.hpp
            src/common/Returners.cpp
            src/common/Previews.hpp
            src/common/Previews.cpp
)

target_compile_options(${CommonTargetName} PUBLIC /bigobj)

target_link_libraries(${CommonTargetName}
        PUBLIC
            AllLibraries
            config
            level
)  

target_include_directories(${CommonTargetName}
        PUBLIC
            src
)