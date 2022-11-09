set(ConfigTargetName config)
set(ConfigFileName ${CMAKE_PROJECT_NAME}Config.hpp)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config/${ConfigFileName}.in ${ConfigFileName})

project(${ConfigTargetName})

add_library(${ConfigTargetName} INTERFACE
            src/config/${ConfigFileName}.in
            ${PROJECT_BINARY_DIR}/${ConfigFileName}
)

target_include_directories(${ConfigTargetName}
    INTERFACE
        ${PROJECT_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

