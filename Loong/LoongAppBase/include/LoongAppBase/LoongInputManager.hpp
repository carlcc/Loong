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

#include <gainput/gainput.h>

namespace Loong {

using namespace gainput;

class LoongInputManagerBase {
public:
    LoongInputManagerBase()
    {
        auto mouseId = inputManager_.CreateDevice<gainput::InputDeviceMouse>();
        auto keyboardId = inputManager_.CreateDevice<gainput::InputDeviceKeyboard>();
        auto padId = inputManager_.CreateDevice<gainput::InputDevicePad>();
        mouseDevice_ = inputManager_.GetDevice(mouseId);
        keyboardDevice_ = inputManager_.GetDevice(keyboardId);
        padDevice_ = inputManager_.GetDevice(padId);
    }

    void BeginFrame()
    {
        inputManager_.Update();
    }

    const gainput::InputDevice& GetKeyboard() const
    {
        return *keyboardDevice_;
    }

    const gainput::InputDevice& GetMouse() const
    {
        return *mouseDevice_;
    }

    const gainput::InputDevice& GetPad() const
    {
        return *padDevice_;
    }

protected:
    gainput::InputManager inputManager_;
    gainput::InputDevice* mouseDevice_;
    gainput::InputDevice* keyboardDevice_;
    gainput::InputDevice* padDevice_;
};

} // namespace Loong

// clang-format off
#if PLATFORM_WIN32
    #include "LoongAppBase/Win32/LoongInputManagerWin32.hpp"
    namespace Loong
    {
        using LoongInputManager = LoongInputManagerWin32;
    }
#elif PLATFORM_MACOS
    #include "LoongAppBase/MacOS/LoongInputManagerMacOS.hpp"
    namespace Loong
    {
        using LoongInputManager = LoongInputManagerMacOS;
    }
#elif PLATFORM_LINUX
    #include "LoongAppBase/Linux/LoongInputManagerLinux.hpp"
    namespace Loong
    {
        using LoongInputManager = LoongInputManagerLinux;
    }
#else
    static_assert(false, "Unsupported platform");
#endif
// clang-format on