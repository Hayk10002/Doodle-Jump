set(MainTargetName ${CMAKE_PROJECT_NAME})

add_executable(${MainTargetName} main.cpp)

target_link_libraries(${MainTargetName} 
        PRIVATE
            config
            AllLibraries
)

if(USE_SFML)
    include("${AllLibrariesFolderPath}/${SFMLFolderName}/CopySFMLDlls.cmake")
    copySFMLDebugDlls(Debug)
    copySFMLReleaseDlls(Release)
    copySFMLReleaseDlls(MinSizeRel)
    copySFMLReleaseDlls(RelWithDebInfo)
endif()