cmake_minimum_required(VERSION 3.2)

project(TestDiligentGlfw CXX)

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
            Diligent-GraphicsEngineMetal-static
            Diligent-GraphicsEngineMetalInterface
            Diligent-GraphicsEngineVk-static
            Diligent-GraphicsEngineVkInterface
            )
    set(LOONG_PLATFORM_SRC_FILES platform/macos/GetNativeWindow.mm)
else()
    #message(FATAL_ERROR "Unkown system")
    # message(FATAL_ERROR "Unkown system")
    set(LOONG_PLATFORM_GRAPHICS_IMPLS
        # Metal has not been implemented
        Diligent-GraphicsEngineVk-static
        Diligent-GraphicsEngineVkInterface
    )
    set(LOONG_PLATFORM_SRC_FILES platform/linux/GetNativeWindow.cpp)
endif()

set(SOURCE
        src/main.cpp
        ${LOONG_PLATFORM_SRC_FILES}
        )

set(INCLUDE
        platform/GetNativeWindow.h
        )

set(ASSETS

        )

add_executable(TestDiligentGlfw ${SOURCE} ${INCLUDE})

target_link_libraries(TestDiligentGlfw
PRIVATE
        LoongFoundation
        Diligent-BuildSettings
        Diligent-GraphicsEngine
        ${LOONG_PLATFORM_GRAPHICS_IMPLS}
        Diligent-Common
        Diligent-GraphicsTools
        # Diligent-TextureLoader
        Diligent-TargetPlatform
        # Diligent-Imgui
        glfw
        )