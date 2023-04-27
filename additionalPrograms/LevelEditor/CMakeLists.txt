set(LevelEditorTargetName LevelEditor)

add_executable(${LevelEditorTargetName} main.cpp)

target_link_libraries(${LevelEditorTargetName} 
        PRIVATE
            config
            AllLibraries
            gameObjects
            drawables
            level
            common
)

set_target_properties(${LevelEditorTargetName} PROPERTIES FOLDER "additionalPrograms")

if(USE_SFML)
    include("${AllLibrariesFolderPath}/${SFMLFolderName}/CopySFMLDlls.cmake")
    copySFMLDebugDlls(Debug)
    copySFMLReleaseDlls(Release)
    copySFMLReleaseDlls(MinSizeRel)
    copySFMLReleaseDlls(RelWithDebInfo)
endif()