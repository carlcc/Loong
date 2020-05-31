/*
 *  Copyright 2019-2020 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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

#pragma once

#include <vector>

#include "BasicMath.hpp"
#include "DeviceContext.h"
#include "EngineFactory.h"
#include "LoongAppBase/LoongInputManager.hpp"
#include "RefCntAutoPtr.hpp"
#include "RenderDevice.h"
#include "SwapChain.h"

#include <gainput/gainput.h>

namespace Diligent {

class ImGuiImplDiligent;

struct SampleInitInfo {
    IEngineFactory* pEngineFactory = nullptr;
    IRenderDevice* pDevice = nullptr;
    IDeviceContext** ppContexts = nullptr;
    Uint32 NumDeferredCtx = 0;
    ISwapChain* pSwapChain = nullptr;
    ImGuiImplDiligent* pImGui = nullptr;
};

class LoongApplication {
public:
    virtual ~LoongApplication() {}

    virtual void GetEngineInitializationAttribs(RENDER_DEVICE_TYPE DeviceType, EngineCreateInfo& EngineCI, SwapChainDesc& SCDesc);

    virtual void Initialize(const SampleInitInfo& InitInfo) = 0;

    virtual void Render() = 0;
    virtual void Update(double CurrTime, double ElapsedTime) = 0;
    virtual void WindowResize(Uint32 Width, Uint32 Height) {}
    virtual bool HandleNativeMessage(const void* pNativeMsgData) { return false; }

    virtual const Char* GetSampleName() const { return "Diligent Engine Sample"; }
    virtual void ProcessCommandLine(const char* CmdLine) {}

    LoongInputManager& GetInputManager()
    {
        return inputManager_;
    }

    void ResetSwapChain(ISwapChain* pNewSwapChain)
    {
        m_pSwapChain = pNewSwapChain;
    }

protected:
    // Returns projection matrix adjusted to the current screen orientation
    float4x4 GetAdjustedProjectionMatrix(float FOV, float NearPlane, float FarPlane) const;

    // Returns pretransform matrix that matches the current screen rotation
    float4x4 GetSurfacePretransformMatrix(const float3& f3CameraViewAxis) const;

    RefCntAutoPtr<IEngineFactory> m_pEngineFactory;
    RefCntAutoPtr<IRenderDevice> m_pDevice;
    RefCntAutoPtr<IDeviceContext> m_pImmediateContext;
    std::vector<RefCntAutoPtr<IDeviceContext>> m_pDeferredContexts;
    RefCntAutoPtr<ISwapChain> m_pSwapChain;
    ImGuiImplDiligent* m_pImGui = nullptr;

    float m_fSmoothFPS = 0;
    double m_LastFPSTime = 0;
    Uint32 m_NumFramesRendered = 0;
    Uint32 m_CurrentFrameNumber = 0;

    LoongInputManager inputManager_;
};

inline void LoongApplication::Update(double CurrTime, double ElapsedTime)
{
    ++m_NumFramesRendered;
    ++m_CurrentFrameNumber;
    static const double dFPSInterval = 0.5;
    if (CurrTime - m_LastFPSTime > dFPSInterval) {
        m_fSmoothFPS = static_cast<float>(m_NumFramesRendered / (CurrTime - m_LastFPSTime));
        m_NumFramesRendered = 0;
        m_LastFPSTime = CurrTime;
    }
}

inline void LoongApplication::Initialize(const SampleInitInfo& InitInfo)
{
    m_pEngineFactory = InitInfo.pEngineFactory;
    m_pDevice = InitInfo.pDevice;
    m_pSwapChain = InitInfo.pSwapChain;
    m_pImmediateContext = InitInfo.ppContexts[0];
    m_pDeferredContexts.resize(InitInfo.NumDeferredCtx);
    for (Uint32 ctx = 0; ctx < InitInfo.NumDeferredCtx; ++ctx)
        m_pDeferredContexts[ctx] = InitInfo.ppContexts[1 + ctx];
    m_pImGui = InitInfo.pImGui;
}

extern LoongApplication* CreateLoongApplication();

} // namespace Diligent
