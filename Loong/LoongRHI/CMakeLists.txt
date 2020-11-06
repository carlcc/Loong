cmake_minimum_required (VERSION 3.2)

project(LoongRHI)


if (WIN32)
    set(LOONG_PLATFORM_GRAPHICS_IMPLS
        Diligent-GraphicsEngineD3D11-static
        Diligent-GraphicsEngineD3D11Interface
        Diligent-GraphicsEngineD3D12-static
        Diligent-GraphicsEngineD3D12Interface
        Diligent-GraphicsEngineVk-static
        Diligent-GraphicsEngineVkInterface
    )
    set(LOONG_PLATFORM_SRC_FILES platform/win/GetNativeWindow.cpp)
elseif (APPLE)
    set(LOONG_PLATFORM_GRAPHICS_IMPLS
        # Metal has not been implemented
        # Diligent-GraphicsEngineMetal-static
        # Diligent-GraphicsEngineMetalInterface
        Diligent-GraphicsEngineVk-static
        Diligent-GraphicsEngineVkInterface
    )
    set(LOONG_PLATFORM_SRC_FILES platform/macos/GetNativeWindow.mm)
else()
    # message(FATAL_ERROR "Unkown system")
    set(LOONG_PLATFORM_GRAPHICS_IMPLS
        # Metal has not been implemented
        Diligent-GraphicsEngineVk-static
        Diligent-GraphicsEngineVkInterface
    )
    set(LOONG_PLATFORM_SRC_FILES platform/linux/GetNativeWindow.cpp)
endif()


file(GLOB_RECURSE SOURCE src/*)
file(GLOB_RECURSE INCLUDE include/*)

add_library(LoongRHI STATIC
        ${SOURCE}
        ${INCLUDE}
        ${LOONG_PLATFORM_SRC_FILES}
        platform/GetNativeWindow.h
)

target_include_directories(LoongRHI
PUBLIC
    include

PRIVATE:
    platform
)

target_link_libraries(LoongRHI
PUBLIC
    LoongFoundation
    Diligent-BuildSettings
    Diligent-GraphicsEngine
    ${LOONG_PLATFORM_GRAPHICS_IMPLS}
    Diligent-Common
    Diligent-GraphicsTools
    Diligent-TargetPlatform
    Diligent-TextureLoader
    glfw
)

source_group("src" FILES ${SOURCE} ${LOONG_PLATFORM_SRC_FILES})
source_group("include" FILES ${INCLUDE} platform/GetNativeWindow.h)

set_target_properties(LoongRHI PROPERTIES
    FOLDER Loong
)
