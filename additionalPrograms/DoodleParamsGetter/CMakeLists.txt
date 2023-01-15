set(DoodleParamsGetterTargetName DoodleParamsGetter)

add_executable(${DoodleParamsGetterTargetName} main.cpp)

target_link_libraries(${DoodleParamsGetterTargetName} 
        PRIVATE
            config
            AllLibraries
            gameObjects
            drawables
            level
            common
)

set_target_properties(${DoodleParamsGetterTargetName} PROPERTIES FOLDER "additionalPrograms")

if(USE_SFML)
    include("${AllLibrariesFolderPath}/${SFMLFolderName}/CopySFMLDlls.cmake")
    copySFMLDebugDlls(Debug)
    copySFMLReleaseDlls(Release)
    copySFMLReleaseDlls(MinSizeRel)
    copySFMLReleaseDlls(RelWithDebInfo)
endif()