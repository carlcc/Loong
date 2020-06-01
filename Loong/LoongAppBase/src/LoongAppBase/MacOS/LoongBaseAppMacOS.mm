/*     Copyright 2015-2019 Egor Yusov
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
*
*  In no event and under no legal theory, whether in tort (including negligence),
*  contract, or otherwise, unless required by applicable law (such as deliberate
*  and grossly negligent acts) or agreed to in writing, shall any Contributor be
*  liable for any damages, including any direct, indirect, special, incidental,
*  or consequential damages of any character arising as a result of this License or
*  out of the use or inability to use the software (including but not limited to damages
*  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
*  all other commercial damages or losses), even if such Contributor has been advised
*  of the possibility of such damages.
*/

#include <queue>
#include <mutex>
#import <Cocoa/Cocoa.h>
#include "LoongAppBase/LoongBaseApp.hpp"
#include "ImGuiImplMacOS.hpp"

namespace Loong
{
using namespace Diligent;

class LoongBaseAppMacOS final : public LoongBaseApp
{
public:
    LoongBaseAppMacOS()
    {
        m_DeviceType = RENDER_DEVICE_TYPE_GL;
    }

    virtual void Initialize(void* view)override final
    {
        m_DeviceType = view == nullptr ? RENDER_DEVICE_TYPE_GL : RENDER_DEVICE_TYPE_VULKAN;
        MacOSNativeWindow MacWindow{view};
        InitializeDiligentEngine(&MacWindow);
        const auto& SCDesc = m_pSwapChain->GetDesc();
        m_pImGui.reset(new ImGuiImplMacOS(m_pDevice, SCDesc.ColorBufferFormat, SCDesc.DepthBufferFormat));
        InitializeSample();
    }

    virtual void Render()override
    {
        std::lock_guard<std::mutex> lock(AppMutex);

        m_pImmediateContext->SetRenderTargets(0, nullptr, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        LoongBaseApp::Render();
    }

    virtual void Update(double CurrTime, double ElapsedTime)override
    {
        std::lock_guard<std::mutex> lock(AppMutex);
        LoongBaseApp::Update(CurrTime, ElapsedTime);
    }

    virtual void WindowResize(int width, int height)override
    {
        std::lock_guard<std::mutex> lock(AppMutex);
        LoongBaseApp::WindowResize(width, height);
    }

    virtual void Present()override
    {
        std::lock_guard<std::mutex> lock(AppMutex);
        LoongBaseApp::Present();
    }

    virtual void HandleOSXEvent(void* _event, void* _view)override final
    {
        auto* event = (NSEvent*)_event;

        switch(event.type)
        {
            case NSEventTypeLeftMouseDown:
            case NSEventTypeRightMouseDown:
            case NSEventTypeLeftMouseUp:
            case NSEventTypeRightMouseUp:
            case NSEventTypeOtherMouseDown:
            case NSEventTypeOtherMouseUp:
            case NSEventTypeMouseMoved:
            case NSEventTypeLeftMouseDragged:
            case NSEventTypeRightMouseDragged:
            case NSEventTypeKeyDown:
            case NSEventTypeKeyUp:
            case NSEventTypeFlagsChanged:
            case NSEventTypeScrollWheel:
                break;
            
            default:
                return;
        }

        std::lock_guard<std::mutex> lock(AppMutex);


        auto* view  = (NSView*)_view;
        if (!static_cast<ImGuiImplMacOS*>(m_pImGui.get())->HandleOSXEvent(event, view))
        {

        }
    }

private:
    // Render functions are called from high-priority Display Link thread,
    // so all methods must be protected by mutex
    std::mutex AppMutex;
};

}

namespace Diligent {

NativeAppBase* CreateApplication()
{
    return new Loong::LoongBaseAppMacOS;
}

}
