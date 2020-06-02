//
// Copyright (c) carlcc. All rights reserved.
//
#pragma once

#include "LoongGui/LoongGuiElement.h"
#include <vector>

namespace Loong::Gui {

class LoongGuiContainer : public LoongGuiElement {

    enum ContainerFlag : uint32_t {
        kClosable = 0x01u,
    };

public:
    explicit LoongGuiContainer(const std::string& label)
        : LoongGuiElement(label)
    {
        SetClosable(true);
    }

    ~LoongGuiContainer() override;

    void ClearChildren();

    template <class T>
    T* CreateChild(const std::string& label = "")
    {
        static_assert(std::is_base_of_v<T, LoongGuiElement>, "The created child must be a instance of LoongGuiElement");
        auto* c = new T(label);
        AddChild(c);
        return c;
    }

    void AddChild(LoongGuiElement* child) { children_.push_back(child); }

    LOONG_GUI_DEFINE_FLAG_GETTER_SETTER(Closable, containerFlags_, kClosable)

protected:
    void DrawChildren();

protected:
    std::vector<LoongGuiElement*> children_ {};
    uint32_t containerFlags_ { 0 };
};

}
