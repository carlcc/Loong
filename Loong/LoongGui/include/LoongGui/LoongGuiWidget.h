//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMacros.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongFoundation/LoongSigslotHelper.h"
#include <atomic>
#include <string>

namespace Loong::Gui {

class LoongGuiContainer;

struct LoongGuiTypeInfo {
    const char* typeName;
    const LoongGuiTypeInfo* parent;
};

#define LOONG_GUI_OBJECT(Class, className, Base)                                                \
public:                                                                                         \
    static const char* GetTypeNameStatic() { return GetTypeInfoStatic().typeName; }             \
    const char* GetTypeName() const override { return Class::GetTypeNameStatic(); }             \
    const LoongGuiTypeInfo& GetTypeInfo() const override { return Class::GetTypeInfoStatic(); } \
    static const LoongGuiTypeInfo& GetTypeInfoStatic()                                          \
    {                                                                                           \
        static LoongGuiTypeInfo sTypeInfo { className, &Base::GetTypeInfoStatic() };            \
        return sTypeInfo;                                                                       \
    }                                                                                           \
    static bool IsInstance(const LoongGuiWidget* w)                                             \
    {                                                                                           \
        const auto* typeInfo = &w->GetTypeInfo();                                               \
        do {                                                                                    \
            if (&Class::GetTypeInfoStatic() == typeInfo) {                                      \
                return true;                                                                    \
            }                                                                                   \
            typeInfo = typeInfo->parent;                                                        \
        } while (typeInfo != nullptr);                                                          \
        return false;                                                                           \
    }                                                                                           \
                                                                                                \
private:

class LoongGuiWidget {
public:
    explicit LoongGuiWidget();

    void SetLabel(const std::string& label);

    const std::string& GetLabel() { return label_; }

    void SetParent(LoongGuiContainer* parent);

    LG_NODISCARD LoongGuiContainer* GetParent() const { return parent_; }

    LG_NODISCARD bool HasParent() const { return GetParent() != nullptr; }

    virtual void Draw();

    void SetName(const std::string& name) { name_ = name; }

    LG_NODISCARD const std::string& GetName() const { return name_; }

    void SetSize(const Math::Vector2& size)
    {
        size_ = size;
        sizeChangedByApi_ = true;
    }

    LG_NODISCARD const Math::Vector2& GetSize() const { return size_; }

    void SetPosition(const Math::Vector2& pos)
    {
        position_ = pos;
        posChangedByApi_ = true;
    }

    LG_NODISCARD const Math::Vector2& GetPosition() const { return position_; }

    //==== Reflection
    virtual const LoongGuiTypeInfo& GetTypeInfo() const { return LoongGuiWidget::GetTypeInfoStatic(); }

    static const LoongGuiTypeInfo& GetTypeInfoStatic()
    {
        static LoongGuiTypeInfo sTypeInfo { "Widget", nullptr };
        return sTypeInfo;
    }

    virtual const char* GetTypeName() const { return LoongGuiWidget::GetTypeNameStatic(); }

    static const char* GetTypeNameStatic() { return GetTypeInfoStatic().typeName; }

    static bool IsInstance(const LoongGuiWidget* w)
    {
        auto& typeInfo = w->GetTypeInfo();
        return &LoongGuiWidget::GetTypeInfoStatic() == &typeInfo;
    }
    template <class T>
    bool IsInstanceOf() const { return T::IsInstance(this); }
    //===========

protected:
    virtual void DrawThis() = 0;
    virtual ~LoongGuiWidget() = default;

    friend class LoongGuiContainer;

protected:
    static std::atomic_int64_t sIdCounter;

    const std::string id_ {};
    std::string label_ {};
    std::string labelAndId_ {};
    LoongGuiContainer* parent_ { nullptr };

    std::string name_ {};
    bool isEnabled_ { true };
    bool hasLineBreak_ { true };

    Math::Vector2 position_ { 0.F, 0.F };
    Math::Vector2 size_ { 100.F, 100.F };
    // Used to tell the draw method whether it should tell imgui to use a new position or size
    bool sizeChangedByApi_ { false };
    bool posChangedByApi_ { false };
};
}