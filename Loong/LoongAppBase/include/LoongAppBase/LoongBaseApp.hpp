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

#include <memory>
#include <string>
#include <vector>

#include "DeviceContext.h"
#include "EngineFactory.h"
#include "Image.h"
#include "LoongAppBase/LoongApplication.hpp"
#include "NativeAppBase.hpp"
#include "RefCntAutoPtr.hpp"
#include "RenderDevice.h"
#include "ScreenCapture.hpp"
#include "SwapChain.h"

namespace Diligent {
class ImGuiImplDiligent;
}

namespace Loong {

class LoongBaseApp : public Diligent::NativeAppBase {
public:
    LoongBaseApp();
    ~LoongBaseApp();
    virtual void ProcessCommandLine(const char* CmdLine) override final;
    virtual const char* GetAppTitle() const override final { return m_AppTitle.c_str(); }
    virtual void Update(double CurrTime, double ElapsedTime) override;
    virtual void WindowResize(int width, int height) override;
    virtual void Render() override;
    virtual void Present() override;
    virtual void SelectDeviceType() {};

    virtual void GetDesiredInitialWindowSize(int& width, int& height) override final
    {
        width = initialWindowWidth_;
        height = initialWindowHeight_;
    }

    virtual GoldenImageMode GetGoldenImageMode() const override final
    {
        return goldenImgMode_;
    }

    virtual int GetExitCode() const override final
    {
        return exitCode_;
    }

protected:
    void InitializeDiligentEngine(const Diligent::NativeWindow* pWindow);
    void InitializeSample();
    void UpdateAdaptersDialog();

    virtual void SetFullscreenMode(const Diligent::DisplayModeAttribs& DisplayMode)
    {
        isFullScreenMode_ = true;
        m_pSwapChain->SetFullscreenMode(DisplayMode);
    }
    virtual void SetWindowedMode()
    {
        isFullScreenMode_ = false;
        m_pSwapChain->SetWindowedMode();
    }

    void CompareGoldenImage(const std::string& FileName, Diligent::ScreenCapture::CaptureInfo& Capture);
    void SaveScreenCapture(const std::string& FileName, Diligent::ScreenCapture::CaptureInfo& Capture);

    Diligent::RENDER_DEVICE_TYPE m_DeviceType = Diligent::RENDER_DEVICE_TYPE_UNDEFINED;
    Diligent::RefCntAutoPtr<Diligent::IEngineFactory> m_pEngineFactory;
    Diligent::RefCntAutoPtr<Diligent::IRenderDevice> m_pDevice;
    Diligent::RefCntAutoPtr<Diligent::IDeviceContext> m_pImmediateContext;
    std::vector<Diligent::RefCntAutoPtr<Diligent::IDeviceContext>> m_pDeferredContexts;
    Diligent::RefCntAutoPtr<Diligent::ISwapChain> m_pSwapChain;
    Diligent::AdapterAttribs m_AdapterAttribs;
    std::vector<Diligent::DisplayModeAttribs> m_DisplayModes;

    std::unique_ptr<LoongApplication> m_TheSample;

    int initialWindowWidth_ = 0;
    int initialWindowHeight_ = 0;
    int m_ValidationLevel = -1;
    std::string m_AppTitle;
    Diligent::Uint32 m_AdapterId = 0;
    Diligent::ADAPTER_TYPE m_AdapterType = Diligent::ADAPTER_TYPE_UNKNOWN;
    std::string m_AdapterDetailsString;
    int m_SelectedDisplayMode = 0;
    bool m_bVSync = false;
    bool isFullScreenMode_ = false;
    bool m_bShowAdaptersDialog = true;
    bool m_bShowUI = true;
    double m_CurrentTime = 0;
    Diligent::Uint32 m_MaxFrameLatency = Diligent::SwapChainDesc {}.BufferCount;

    // We will need this when we have to recreate the swap chain (on Android)
    Diligent::SwapChainDesc m_SwapChainInitDesc;

    struct ScreenCaptureInfo {
        bool AllowCapture = false;
        std::string Directory;
        std::string FileName = "frame";
        double CaptureFPS = 30;
        double LastCaptureTime = -1e+10;
        Diligent::Uint32 FramesToCapture = 0;
        Diligent::Uint32 CurrentFrame = 0;
        Diligent::IMAGE_FILE_FORMAT FileFormat = Diligent::IMAGE_FILE_FORMAT_PNG;
        int JpegQuality = 95;
        bool KeepAlpha = false;

    } m_ScreenCaptureInfo;
    std::unique_ptr<Diligent::ScreenCapture> m_pScreenCapture;

    std::unique_ptr<Diligent::ImGuiImplDiligent> m_pImGui;

    GoldenImageMode goldenImgMode_ = GoldenImageMode::None;
    int m_GoldenImgPixelTolerance = 0;
    int exitCode_ = 0;
};

} // namespace Loong
