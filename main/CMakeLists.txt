set(TargetName ${CMAKE_PROJECT_NAME})

add_executable(${TargetName} main.cpp)

target_link_libraries(${TargetName} 
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