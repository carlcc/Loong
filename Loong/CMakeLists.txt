if (WIN32)
    set(LOONG_WIN32_FLAG WIN32)
endif()

add_subdirectory(LoongFoundation)
add_subdirectory(LoongAppBase)
add_subdirectory(LoongGui)
add_subdirectory(LoongFileSystem)
add_subdirectory(PlayGround)