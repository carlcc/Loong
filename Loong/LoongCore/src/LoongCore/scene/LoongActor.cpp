//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongCore/scene/LoongActor.h"
#include "LoongCore/scene/LoongScene.h"
#include "LoongFoundation/LoongLogger.h"
#include <algorithm>

namespace Loong::Core {

LoongActor::LoongActor(uint32_t actorID, std::string name, std::string tag)
    : name_(std::move(name))
    , tag_(std::move(tag))
    , actorID_(actorID)
{
}

LoongActor::~LoongActor()
{
    LOONG_TRACE("Destruct actor {}(ID {})", name_, actorID_);
    DestroySignal_.emit(this);

    std::vector<LoongActor*> toDetach = children_;

    for (auto child : toDetach) {
        child->DetachFromParent();
    }

    toDetach.clear();

    DetachFromParent();

    std::for_each(components_.begin(), components_.end(), [&](const std::shared_ptr<LoongComponent>& component) { RemoveComponentSignal_.emit(this, component.get()); });
    std::for_each(children_.begin(), children_.end(), [](LoongActor* child) { delete child; });
}

void LoongActor::SetActive(bool active)
{
    if (isActive_ != active) {
        RecursiveWasActiveUpdate();
        isActive_ = active;
        RecursiveActiveUpdate();
    }
}

void LoongActor::SetParent(LoongActor* parent)
{
    if (parent_ == parent) {
        return;
    }

    DetachFromParent();

    parent_ = parent;
    if (parent_ != nullptr) {
        parent_->GetChildren().push_back(this);
        transform_.SetParent(&parent->transform_);

        // If the root node is a LoongScene object, we update its FastAccess
        if (auto* root = dynamic_cast<LoongScene*>(GetRoot()); root != nullptr) {
            root->RecursiveAddToFastAccess(this);
        }
        if (auto* scene = dynamic_cast<LoongScene*>(this); scene != nullptr) {
            scene->fastAccess_.Clear();
        }
    } else {
        transform_.SetParent(nullptr);
    }

    AttachToParentSignal_.emit(this, parent);
}

void LoongActor::DetachFromParent()
{
    if (parent_ != nullptr) {
        if (auto* scene = dynamic_cast<LoongScene*>(this); scene != nullptr) {
            scene->ConstructFastAccess();
        }
        // If the root node is a LoongScene object, we update its FastAccess
        if (auto* root = dynamic_cast<LoongScene*>(GetRoot()); root != nullptr) {
            root->RecursiveRemoveFromFastAccess(this);
        }
        DetachFromParentSignal_.emit(this, parent_);
        auto& parentChildren = parent_->GetChildren();

        auto it = std::find(parentChildren.begin(), parentChildren.end(), this);
        assert(it != parentChildren.end());
        parentChildren.erase(it);

        parent_ = nullptr;
    }
}

template <class PropertyGetter, class ValueT>
LoongActor* FindChildRecursive(const LoongActor* actor, const ValueT& value)
{
    for (auto* child : actor->GetChildren()) {
        if (PropertyGetter::GetProperty(child) == value) {
            return child;
        }
        auto* tmp = FindChildRecursive<PropertyGetter, ValueT>(child, value);
        if (tmp != nullptr) {
            return tmp;
        }
    }
    return nullptr;
}

template <class PropertyGetter, class ValueT>
LoongActor* FindChild(const LoongActor* actor, const ValueT& value)
{
    for (auto* child : actor->GetChildren()) {
        if (PropertyGetter::GetProperty(child) == value) {
            return child;
        }
    }
    return nullptr;
}

struct ActorNameGetter {
    static const std::string& GetProperty(const LoongActor* actor) { return actor->GetName(); }
};

LoongActor* LoongActor::GetChildByName(const std::string& name) const
{
    return FindChild<ActorNameGetter>(this, name);
}

LoongActor* LoongActor::GetChildByNameRecursive(const std::string& name) const
{
    return FindChildRecursive<ActorNameGetter>(this, name);
}

struct ActorTagGetter {
    static const std::string& GetProperty(const LoongActor* actor) { return actor->GetTag(); }
};

LoongActor* LoongActor::GetChildByTag(const std::string& tag) const
{
    return FindChild<ActorTagGetter>(this, tag);
}

LoongActor* LoongActor::GetChildByTagRecursive(const std::string& tag) const
{
    return FindChildRecursive<ActorTagGetter>(this, tag);
}

void LoongActor::MarkAsDestroy()
{
    isDestroyed_ = true;
    for (auto child : children_) {
        child->MarkAsDestroy();
    }
}

void LoongActor::OnStart()
{
    assert(!isStarted_);
    isStarted_ = true;
    std::for_each(components_.begin(), components_.end(), [](const std::shared_ptr<LoongComponent>& component) { component->OnStart(); });
}

void LoongActor::OnEnable()
{
    std::for_each(components_.begin(), components_.end(), [](const std::shared_ptr<LoongComponent>& component) { component->OnEnable(); });
}

void LoongActor::OnDisable()
{
    std::for_each(components_.begin(), components_.end(), [](const std::shared_ptr<LoongComponent>& component) { component->OnDisable(); });
}

void LoongActor::OnDestroy()
{
    std::for_each(components_.begin(), components_.end(), [](const std::shared_ptr<LoongComponent>& component) { component->OnDestroy(); });
}

void LoongActor::OnUpdate(const Foundation::LoongClock& clock)
{
    std::for_each(components_.begin(), components_.end(), [&clock](const std::shared_ptr<LoongComponent>& component) { component->OnUpdate(clock); });
}

void LoongActor::OnFixedUpdate(const Foundation::LoongClock& clock)
{
    std::for_each(components_.begin(), components_.end(), [&clock](const std::shared_ptr<LoongComponent>& component) { component->OnFixedUpdate(clock); });
}

void LoongActor::OnLateUpdate(const Foundation::LoongClock& clock)
{
    std::for_each(components_.begin(), components_.end(), [&clock](const std::shared_ptr<LoongComponent>& component) { component->OnLateUpdate(clock); });
}

bool LoongActor::RemoveComponent(LoongComponent* component)
{
    for (auto it = components_.begin(); it != components_.end(); ++it) {
        if (it->get() == component) {
            RemoveComponentSignal_.emit(this, component);
            components_.erase(it);
            return true;
        }
    }
    return false;
}

void LoongActor::RecursiveActiveUpdate()
{
    bool isActive = IsActive();

    if (!wasActive_ && isActive) {
        OnEnable();

        if (!isStarted_) {
            OnStart();
        }
    }

    if (wasActive_ && !isActive) {
        OnDisable();
    }

    for (auto child : children_) {
        child->RecursiveActiveUpdate();
    }
}

void LoongActor::RecursiveWasActiveUpdate()
{
    wasActive_ = IsActive();
    for (auto child : children_) {
        child->RecursiveWasActiveUpdate();
    }
}

}
