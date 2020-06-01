cmake_minimum_required (VERSION 3.6)

project(LoonAppBase)

if(PLATFORM_WIN32)
    set(SOURCE
        src/LoongAppBase/Win32/LoongBaseAppWin32.cpp
        src/LoongAppBase/Win32/LoongInputManagerWin32.cpp
        src/LoongAppBase/LoongBaseApp.cpp
    )
    set(INCLUDE
        include/LoongAppBase/LoongBaseApp.hpp
        include/LoongAppBase/Win32/LoongInputManagerWin32.hpp
    )
    set(WIN32_RESOURCES
        src/LoongAppBase/Win32/resources/Win32AppResource.h
        src/LoongAppBase/Win32/resources/directx11-logo.bmp
        src/LoongAppBase/Win32/resources/directx12-logo.bmp
        src/LoongAppBase/Win32/resources/vulkan-logo.bmp
        src/LoongAppBase/Win32/resources/opengl-logo.bmp
        src/LoongAppBase/Win32/resources/Win32AppResource.rc
    )

    function(append_loong_app_base_win32_source TARGET_NAME)
        get_target_property(LOONG_APP_BASE_SOURCE_DIR LoonAppBase SOURCE_DIR)
        # We need to add .res file to executable target to make resources available
        set(RES_FILE ${LOONG_APP_BASE_SOURCE_DIR}/src/LoongAppBase/Win32/resources/Win32AppResource.rc)
        target_sources(${TARGET_NAME} PRIVATE ${RES_FILE})
        source_group("resources" FILES ${RES_FILE})
    endfunction()
elseif(PLATFORM_LINUX)
    set(SOURCE 
        src/LoongAppBase/Linux/LoongInputManagerLinux.cpp
        src/LoongAppBase/Linux/LoongBaseAppLinux.cpp
        src/LoongAppBase/LoongBaseApp.cpp
    )
    set(INCLUDE 
        include/LoongAppBase/Linux/LoongInputManagerLinux.hpp
        include/LoongAppBase/LoongBaseApp.hpp
    )
elseif(PLATFORM_MACOS)

    set(SOURCE
        src/LoongAppBase/MacOS/LoongInputManagerMacOS.cpp
        src/LoongAppBase/MacOS/LoongBaseAppMacOS.mm
        src/LoongAppBase/LoongBaseApp.cpp
    )
    set(INCLUDE
        include/LoongAppBase/MacOS/LoongInputManagerMacOS.hpp
        include/LoongAppBase/LoongBaseApp.hpp
    )
endif()

list(APPEND SOURCE
    src/LoongAppBase/LoongApplication.cpp
)

list(APPEND INCLUDE
    include/LoongAppBase/LoongInputManager.hpp
    include/LoongAppBase/LoongApplication.hpp
)


add_library(LoonAppBase STATIC ${SOURCE} ${INCLUDE})
set_loong_common_target_properties(LoonAppBase)

target_include_directories(LoonAppBase
PUBLIC
    include
)

if(MSVC)
    target_compile_options(LoonAppBase PRIVATE -DUNICODE)
endif()

get_supported_backends(ENGINE_LIBRARIES)

target_link_libraries(LoonAppBase 
PUBLIC
    Diligent-BuildSettings

    Diligent-Common
    Diligent-GraphicsTools
    Diligent-TextureLoader
    Diligent-TargetPlatform
    Diligent-Imgui
    ${ENGINE_LIBRARIES}
    Diligent-NativeAppBase
    gainput
)

if(PLATFORM_UNIVERSAL_WINDOWS)
    target_link_libraries(LoonAppBase PRIVATE dxguid.lib)
elseif(PLATFORM_LINUX)
    target_link_libraries(LoonAppBase PRIVATE XCBKeySyms GL X11)
elseif(PLATFORM_MACOS)

endif()

source_group("src" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE})

if(PLATFORM_WIN32)
    target_sources(LoonAppBase PRIVATE ${WIN32_RESOURCES})
    source_group("resources" FILES ${WIN32_RESOURCES})
endif()

set_target_properties(LoonAppBase PROPERTIES
    FOLDER Loong
)
