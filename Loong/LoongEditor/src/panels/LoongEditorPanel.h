//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongClock.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongFoundation/LoongSigslotHelper.h"

namespace Loong::Editor {

class LoongEditor;
class LoongEditorContext;

struct LoongEditorPanelConfig {
    bool isResizable { true };
    bool isClosable { true };
    bool isMovable { true };
    bool isHideBackground { false };
    bool isForceHorizontalScrollbar { false };
    bool isForceVerticalScrollbar { false };
    bool allowHorizontalScrollbar { false };
    bool isBringToFrontOnFocus { true };
    bool isCollapsable { false };
    bool allowInputs { true };
};

class LoongEditorPanel : public Foundation::LoongHasSlots {
public:
    explicit LoongEditorPanel(LoongEditor* editor, const std::string& name = "", bool opened = true, const LoongEditorPanelConfig& cfg = {});
    LoongEditorPanel(const LoongEditorPanel&) = delete;
    LoongEditorPanel(LoongEditorPanel&&) = delete;
    ~LoongEditorPanel() override = default;
    LoongEditorPanel& operator=(const LoongEditorPanel&) = delete;
    LoongEditorPanel& operator=(LoongEditorPanel&&) = delete;

    void Show();

    void Close();

    void Focus();

    void SetVisible(bool value);

    void Update(const Foundation::LoongClock& clock);

    bool IsVisible() const;

    bool IsHovered() const;

    bool IsFocused() const;

    LoongEditorContext& GetEditorContext();

    LOONG_DECLARE_SIGNAL(OnOpen);
    LOONG_DECLARE_SIGNAL(OnClose);

protected:
    virtual void UpdateImpl(const Foundation::LoongClock& clock) = 0;

protected:
    std::string panelId_ {};
    std::string name_ {};

    Math::Vector2 minSize_ { Math::Zero };
    Math::Vector2 maxSize_ { Math::Zero };

    LoongEditorPanelConfig config_ {};

    LoongEditor* editor_ { nullptr };

private:
    bool isVisible_ { true };
    bool isHovered_ { false };
    bool isFocused_ { false };
};

}