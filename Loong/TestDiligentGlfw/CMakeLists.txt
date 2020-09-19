cmake_minimum_required(VERSION 3.2)

project(TestDiligentGlfw CXX)

set(SOURCE
        src/main.cpp
        )

set(INCLUDE

        )

set(ASSETS

        )

add_executable(TestDiligentGlfw ${SOURCE} ${INCLUDE})

target_link_libraries(TestDiligentGlfw
PRIVATE
        LoongFoundation
        Diligent-BuildSettings
        Diligent-GraphicsEngine
        Diligent-GraphicsEngineD3D11-static
        Diligent-GraphicsEngineD3D11Interface
        Diligent-GraphicsEngineD3D12-static
        Diligent-GraphicsEngineD3D12Interface
        Diligent-GraphicsEngineVk-static
        Diligent-GraphicsEngineVkInterface
        Diligent-GraphicsEngineOpenGL-static
        Diligent-GraphicsEngineOpenGLInterface
        Diligent-Common
        Diligent-GraphicsTools
        # Diligent-TextureLoader
        Diligent-TargetPlatform
        # Diligent-Imgui
        glfw
        )
