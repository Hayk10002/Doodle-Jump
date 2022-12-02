set(CommonTargetName common)

project(${CommonTargetName})

add_library(${CommonTargetName} STATIC
            src/common/Resources.hpp
            src/common/Resources.cpp
            src/common/DebugImGui.hpp
            src/common/DebugImGui.cpp
)

target_link_libraries(${CommonTargetName}
        PUBLIC
            AllLibraries
            config
)  

target_include_directories(${CommonTargetName}
        PUBLIC
            src
)