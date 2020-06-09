//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongCore/scene/LoongActor.h"

namespace Loong::Core {

LoongActor::LoongActor(uint32_t actorID, std::string name, std::string tag)
    : actorID_(actorID)
    , name_(std::move(name))
    , tag_(std::move(tag))
{
}

LoongActor::~LoongActor()
{
    DestroySignal_.emit(this);

    std::vector<LoongActor*> toDetach = children_;

    for (auto child : toDetach) {
        child->DetachFromParent();
    }

    toDetach.clear();

    DetachFromParent();

    std::for_each(components_.begin(), components_.end(), [&](LoongComponent* component) { RemoveComponentSignal_.emit(this, component); });
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
    } else {
        transform_.SetParent(nullptr);
    }

    AttachToParentSignal_.emit(this, parent);
}

void LoongActor::DetachFromParent()
{
    if (parent_ != nullptr) {
        DetachFromParentSignal_.emit(this, parent_);
        auto& parentChildren = parent_->GetChildren();

        auto it = std::find(parentChildren.begin(), parentChildren.end(), this);
        assert(it != parentChildren.end());
        parentChildren.erase(it);

        parent_ = nullptr;
    }
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
    std::for_each(components_.begin(), components_.end(), [](LoongComponent* component) { component->OnStart(); });
}

void LoongActor::OnEnable()
{
    std::for_each(components_.begin(), components_.end(), [](LoongComponent* component) { component->OnEnable(); });
}

void LoongActor::OnDisable()
{
    std::for_each(components_.begin(), components_.end(), [](LoongComponent* component) { component->OnDisable(); });
}

void LoongActor::OnDestroy()
{
    std::for_each(components_.begin(), components_.end(), [](LoongComponent* component) { component->OnDestroy(); });
}

void LoongActor::OnUpdate(const Foundation::LoongClock& clock)
{
    std::for_each(components_.begin(), components_.end(), [&clock](LoongComponent* component) { component->OnUpdate(clock); });
}

void LoongActor::OnFixedUpdate(const Foundation::LoongClock& clock)
{
    std::for_each(components_.begin(), components_.end(), [&clock](LoongComponent* component) { component->OnFixedUpdate(clock); });
}

void LoongActor::OnLateUpdate(const Foundation::LoongClock& clock)
{
    std::for_each(components_.begin(), components_.end(), [&clock](LoongComponent* component) { component->OnLateUpdate(clock); });
}

bool LoongActor::RemoveComponent(LoongComponent* component)
{
    for (auto it = components_.begin(); it != components_.end(); ++it) {
        if (*it == component) {
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
