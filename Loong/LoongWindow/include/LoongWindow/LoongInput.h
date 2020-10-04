//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once
#include "LoongFoundation/LoongMacros.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongWindow/LoongInputAction.h"
#include "LoongWindow/LoongKeyCode.h"
#include "LoongWindow/LoongMouseButton.h"
#include <array>
#include <cstdint>

namespace Loong::Window {

class LoongInput {
public:
    LoongInput();

    // clang-format off
    LG_NODISCARD bool IsKeyPressed(LoongKeyCode keyCode)      const { return isKeyPressed_     [int(keyCode) / 8] & (1u << (uint32_t)keyCode % 8); }
    LG_NODISCARD bool IsKeyPressEvent(LoongKeyCode keyCode)   const { return isKeyPressEvent_  [int(keyCode) / 8] & (1u << (uint32_t)keyCode % 8); }
    LG_NODISCARD bool IsKeyReleaseEvent(LoongKeyCode keyCode) const { return isKeyReleaseEvent_[int(keyCode) / 8] & (1u << (uint32_t)keyCode % 8); }
    LG_NODISCARD bool IsKeyRepeatEvent(LoongKeyCode keyCode)  const { return isKeyRepeatEvent_ [int(keyCode) / 8] & (1u << (uint32_t)keyCode % 8); }

    LG_NODISCARD bool IsMouseButtonPressed(LoongMouseButton button)      const { return isMouseButtonPressed_      & (1u << (uint32_t)button % 8); }
    LG_NODISCARD bool IsMouseButtonPressEvent(LoongMouseButton button)   const { return isMouseButtonPressEvent_   & (1u << (uint32_t)button % 8); }
    LG_NODISCARD bool IsMouseButtonReleaseEvent(LoongMouseButton button) const { return isMouseButtonReleaseEvent_ & (1u << (uint32_t)button % 8); }

    LG_NODISCARD const Math::Vector2& GetMouseDownPosition() const { return mouseDownPosition_; }
    LG_NODISCARD const Math::Vector2& GetMousePosition() const { return mousePosition_; }
    LG_NODISCARD const Math::Vector2& GetMouseDelta()    const { return mouseDelta_; }
    // clang-format on

    void BeginFrame();

    // clang-format off
    void SetIsKeyPressed(LoongKeyCode keyCode)      { isKeyPressed_     [int(keyCode) / 8] |= (1u << (uint32_t)keyCode % 8); }
    void SetIsKeyReleased(LoongKeyCode keyCode)     { isKeyPressed_     [int(keyCode) / 8] &= ~(1u << (uint32_t)keyCode % 8); }
    void SetIsKeyPressEvent(LoongKeyCode keyCode)   { isKeyPressEvent_  [int(keyCode) / 8] |= (1u << (uint32_t)keyCode % 8); }
    void SetIsKeyReleaseEvent(LoongKeyCode keyCode) { isKeyReleaseEvent_[int(keyCode) / 8] |= (1u << (uint32_t)keyCode % 8); }
    void SetIsKeyRepeatEvent(LoongKeyCode keyCode)  { isKeyRepeatEvent_ [int(keyCode) / 8] |= (1u << (uint32_t)keyCode % 8); }
    void SetIsMouseButtonPressed(LoongMouseButton button)      { isMouseButtonPressed_      |= 1u << (uint32_t)button % 8; }
    void SetIsMouseButtonReleased(LoongMouseButton button)     { isMouseButtonPressed_      &= ~(1u << (uint32_t)button % 8); }
    void SetIsMouseButtonPressEvent(LoongMouseButton button)   { isMouseButtonPressEvent_   |= 1u << (uint32_t)button % 8; }
    void SetIsMouseButtonReleaseEvent(LoongMouseButton button) { isMouseButtonReleaseEvent_ |= 1u << (uint32_t)button % 8; }
    void SetMouseDownPosition(float x, float y);
    void SetMousePosition(float x, float y);
    void SetMousePositionOnly(float x, float y);
    // clang-format on

private:
    static const int kKeyCodeBytesCount = (int(LoongKeyCode::kKeyCode_Count) + 7) / 8;

    std::array<uint8_t, kKeyCodeBytesCount> isKeyPressed_ {};
    std::array<uint8_t, kKeyCodeBytesCount> isKeyPressEvent_ {};
    std::array<uint8_t, kKeyCodeBytesCount> isKeyReleaseEvent_ {};
    std::array<uint8_t, kKeyCodeBytesCount> isKeyRepeatEvent_ {};

    uint8_t isMouseButtonPressed_ { 0 };
    uint8_t isMouseButtonPressEvent_ { 0 };
    uint8_t isMouseButtonReleaseEvent_ { 0 };

    Math::Vector2 mouseDownPosition_ { Math::Zero };
    Math::Vector2 mousePosition_ { Math::Zero };
    Math::Vector2 mouseDelta_ { Math::Zero };
};

}