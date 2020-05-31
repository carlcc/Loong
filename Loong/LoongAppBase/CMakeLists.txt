cmake_minimum_required (VERSION 3.6)

project(LoonAppBase)

if(PLATFORM_WIN32)
    set(SOURCE
            src/Win32/LoongBaseAppWin32.cpp
        src/Win32/InputControllerWin32.cpp
            src/LoongBaseApp.cpp
    )
    set(INCLUDE
            include/LoongBaseApp.hpp
        include/Win32/InputControllerWin32.hpp
    )
    set(WIN32_RESOURCES
        src/Win32/resources/Win32AppResource.h
        src/Win32/resources/directx11-logo.bmp
        src/Win32/resources/directx12-logo.bmp
        src/Win32/resources/vulkan-logo.bmp
        src/Win32/resources/opengl-logo.bmp
        src/Win32/resources/Win32AppResource.rc
    )

    function(append_loong_app_base_win32_source TARGET_NAME)
        get_target_property(LOONG_APP_BASE_SOURCE_DIR LoonAppBase SOURCE_DIR)
        # We need to add .res file to executable target to make resources available
        set(RES_FILE ${LOONG_APP_BASE_SOURCE_DIR}/src/Win32/resources/Win32AppResource.rc)
        target_sources(${TARGET_NAME} PRIVATE ${RES_FILE})
        source_group("resources" FILES ${RES_FILE})
    endfunction()
elseif(PLATFORM_LINUX)
    set(SOURCE 
        src/Linux/InputControllerLinux.cpp
            src/Linux/LoongBaseAppLinux.cpp
            src/LoongBaseApp.cpp
    )
    set(INCLUDE 
        include/Linux/InputControllerLinux.hpp
            include/LoongBaseApp.hpp
    )
elseif(PLATFORM_MACOS)

    set(SOURCE
        src/MacOS/InputControllerMacOS.cpp
            src/MacOS/LoongBaseAppMacOS.mm
            src/LoongBaseApp.cpp
    )
    set(INCLUDE
        Include/MacOS/InputControllerMacOS.hpp
            include/LoongBaseApp.hpp
    )
endif()

list(APPEND SOURCE
    src/LoongApplication.cpp
)

list(APPEND INCLUDE
    include/InputController.hpp
    include/LoongApplication.hpp
)


add_library(LoonAppBase STATIC ${SOURCE} ${INCLUDE})
set_common_target_properties(LoonAppBase)

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
