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

#include "BasicTypes.h"
#include "FlagEnum.h"

namespace Diligent {

enum class KeyCode {
    // clang-format off
    kKeySpace                 = 32,
    kKeyApostrophe            = 39,  /* ' */
    kKeyComma                 = 44,  /* , */
    kKeyMinus                 = 45,  /* - */
    kKeyPeriod                = 46,  /* . */
    kKeySlash                 = 47,  /* / */
    kKey0                     = 48,
    kKey1                     = 49,
    kKey2                     = 50,
    kKey3                     = 51,
    kKey4                     = 52,
    kKey5                     = 53,
    kKey6                     = 54,
    kKey7                     = 55,
    kKey8                     = 56,
    kKey9                     = 57,
    kKeySemicolon             = 59,  /* ; */
    kKeyEqual                 = 61,  /* = */
    kKeyA                     = 65,
    kKeyB                     = 66,
    kKeyC                     = 67,
    kKeyD                     = 68,
    kKeyE                     = 69,
    kKeyF                     = 70,
    kKeyG                     = 71,
    kKeyH                     = 72,
    kKeyI                     = 73,
    kKeyJ                     = 74,
    kKeyK                     = 75,
    kKeyL                     = 76,
    kKeyM                     = 77,
    kKeyN                     = 78,
    kKeyO                     = 79,
    kKeyP                     = 80,
    kKeyQ                     = 81,
    kKeyR                     = 82,
    kKeyS                     = 83,
    kKeyT                     = 84,
    kKeyU                     = 85,
    kKeyV                     = 86,
    kKeyW                     = 87,
    kKeyX                     = 88,
    kKeyY                     = 89,
    kKeyZ                     = 90,
    kKeyLeftBracket           = 91,  /* [ */
    kKeyBackslash             = 92,  /* \ */
    kKeyRightBracket          = 93,  /* ] */
    kKeyGraveAccent           = 96,  /* ` */
    kKeyWorld1                = 161, /* non-US #1 */
    kKeyWorld2                = 162, /* non-US #2 */

    /* Function keys */
    kKeyEscape                = 256,
    kKeyEnter                 = 257,
    kKeyTab                   = 258,
    kKeyBackspace             = 259,
    kKeyInsert                = 260,
    kKeyDelete                = 261,
    kKeyRight                 = 262,
    kKeyLeft                  = 263,
    kKeyDown                  = 264,
    kKeyUp                    = 265,
    kKeyPageUp                = 266,
    kKeyPageDown              = 267,
    kKeyHome                  = 268,
    kKeyEnd                   = 269,
    kKeyCapsLock              = 280,
    kKeyScrollLock            = 281,
    kKeyNumLock               = 282,
    kKeyPrintScreen           = 283,
    kKeyPause                 = 284,
    kKeyF1                    = 290,
    kKeyF2                    = 291,
    kKeyF3                    = 292,
    kKeyF4                    = 293,
    kKeyF5                    = 294,
    kKeyF6                    = 295,
    kKeyF7                    = 296,
    kKeyF8                    = 297,
    kKeyF9                    = 298,
    kKeyF10                   = 299,
    kKeyF11                   = 300,
    kKeyF12                   = 301,
    kKeyF13                   = 302,
    kKeyF14                   = 303,
    kKeyF15                   = 304,
    kKeyF16                   = 305,
    kKeyF17                   = 306,
    kKeyF18                   = 307,
    kKeyF19                   = 308,
    kKeyF20                   = 309,
    kKeyF21                   = 310,
    kKeyF22                   = 311,
    kKeyF23                   = 312,
    kKeyF24                   = 313,
    kKeyF25                   = 314,
    kKeyKp0                   = 320,
    kKeyKp1                   = 321,
    kKeyKp2                   = 322,
    kKeyKp3                   = 323,
    kKeyKp4                   = 324,
    kKeyKp5                   = 325,
    kKeyKp6                   = 326,
    kKeyKp7                   = 327,
    kKeyKp8                   = 328,
    kKeyKp9                   = 329,
    kKeyKpDecimal             = 330,
    kKeyKpDivide              = 331,
    kKeyKpMultiply            = 332,
    kKeyKpSubtract            = 333,
    kKeyKpAdd                 = 334,
    kKeyKpEnter               = 335,
    kKeyKpEqual               = 336,
    kKeyLeftShift             = 340,
    kKeyLeftControl           = 341,
    kKeyLeftAlt               = 342,
    kKeyLeftSuper             = 343,
    kKeyRightShift            = 344,
    kKeyRightControl          = 345,
    kKeyRightAlt              = 346,
    kKeyRightSuper            = 347,
    kKeyMenu                  = 348,
    // clang-format on
    kKeyCode_Count
};
enum class MouseButton {
    // clang-format off
    kButtonLeft     = 0x01,
    kButtonRight    = 0x02,
    kButtonMiddle   = 0x04,
    kButton1        = kButtonLeft,
    kButton2        = kButtonRight,
    kButton3        = kButtonMiddle,
    kButton4        = 0x08,
    kButton5        = 0x10,
    kButton6        = 0x20,
    kButton7        = 0x40,
    kButton8        = 0x80,
    // clang-format on
    kButton_Count   = 8
};
DEFINE_FLAG_ENUM_OPERATORS(MouseButton)
struct MouseState {
    enum BUTTON_FLAGS : Uint8 {
        BUTTON_FLAG_NONE = 0x00,
        BUTTON_FLAG_LEFT = 0x01,
        BUTTON_FLAG_MIDDLE = 0x02,
        BUTTON_FLAG_RIGHT = 0x04,
        BUTTON_FLAG_WHEEL = 0x08
    };

    Float32 PosX = -1;
    Float32 PosY = -1;
    BUTTON_FLAGS ButtonFlags = BUTTON_FLAG_NONE;
    Float32 WheelDelta = 0;
};
DEFINE_FLAG_ENUM_OPERATORS(MouseState::BUTTON_FLAGS)

enum class InputKeys {
    Unknown = 0,
    MoveLeft,
    MoveRight,
    MoveForward,
    MoveBackward,
    MoveUp,
    MoveDown,
    Reset,
    ControlDown,
    ShiftDown,
    AltDown,
    ZoomIn,
    ZoomOut,
    TotalKeys
};

enum INPUT_KEY_STATE_FLAGS : Uint8 {
    INPUT_KEY_STATE_FLAG_KEY_NONE = 0x00,
    INPUT_KEY_STATE_FLAG_KEY_IS_DOWN = 0x01,
    INPUT_KEY_STATE_FLAG_KEY_WAS_DOWN = 0x80
};
DEFINE_FLAG_ENUM_OPERATORS(INPUT_KEY_STATE_FLAGS)

class InputControllerBase {
public:
    const MouseState& GetMouseState() const
    {
        return m_MouseState;
    }

    INPUT_KEY_STATE_FLAGS GetKeyState(InputKeys Key) const
    {
        return m_Keys[static_cast<size_t>(Key)];
    }

    bool IsKeyDown(InputKeys Key) const
    {
        return (GetKeyState(Key) & INPUT_KEY_STATE_FLAG_KEY_IS_DOWN) != 0;
    }

    void ClearState()
    {
        m_MouseState.WheelDelta = 0;

        for (Uint32 i = 0; i < static_cast<Uint32>(InputKeys::TotalKeys); ++i) {
            auto& KeyState = m_Keys[i];
            if (KeyState & INPUT_KEY_STATE_FLAG_KEY_WAS_DOWN) {
                KeyState &= ~INPUT_KEY_STATE_FLAG_KEY_WAS_DOWN;
            }
        }
    }

protected:
    MouseState m_MouseState;
    INPUT_KEY_STATE_FLAGS m_Keys[static_cast<size_t>(InputKeys::TotalKeys)] = {};
};

} // namespace Diligent

// clang-format off
#if PLATFORM_WIN32
    #include "Win32/InputControllerWin32.hpp"
    namespace Diligent
    {
        using InputController = InputControllerWin32;
    }
#elif PLATFORM_UNIVERSAL_WINDOWS
    #include "UWP/InputControllerUWP.hpp"
    namespace Diligent
    {
        using InputController = InputControllerUWP;
    }
#elif PLATFORM_MACOS
    #include "MacOS/InputControllerMacOS.hpp"
    namespace Diligent
    {
        using InputController = InputControllerMacOS;
    }
#elif PLATFORM_IOS
    #include "iOS/InputControllerIOS.hpp"
    namespace Diligent
    {
        using InputController = InputControllerIOS;
    }
#elif PLATFORM_LINUX
    #include "Linux/InputControllerLinux.hpp"
    namespace Diligent
    {
        using InputController = InputControllerLinux;
    }
#elif PLATFORM_ANDROID
    #include "Android/InputControllerAndroid.hpp"
    namespace Diligent
    {
        using InputController = InputControllerAndroid;
    }
#else
    namespace Diligent
    {
        class DummyInputController
        {
        public:
            const MouseState& GetMouseState()const{return m_MouseState;}

            INPUT_KEY_STATE_FLAGS GetKeyState(InputKeys Key)const{return INPUT_KEY_STATE_FLAG_KEY_NONE;}

        private:
            MouseState m_MouseState;
        };
        using InputController = DummyInputController;
    }
#endif
// clang-format on