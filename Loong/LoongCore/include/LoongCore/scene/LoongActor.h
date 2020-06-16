//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongCore/scene/LoongComponent.h"
#include "LoongFoundation/LoongClock.h"
#include "LoongFoundation/LoongTransform.h"
#include <cstdint>
#include <string>
#include <vector>

namespace Loong::Core {

class LoongComponent;

class LoongActor {
protected:
    LoongActor(uint32_t actorID, std::string name, std::string tag);

public:
    LoongActor(const LoongActor&) = delete;
    LoongActor(LoongActor&&) = delete;

    virtual ~LoongActor();

    LoongActor& operator=(const LoongActor&) = delete;
    LoongActor& operator=(LoongActor&&) = delete;

    const std::string& GetName() const { return name_; }

    const std::string& GetTag() const { return tag_; }

    void SetName(const std::string& name) { name_ = name; }

    void SetTag(const std::string& tag) { tag_ = tag; }

    void SetActive(bool active);

    bool IsSelfActive() const { return isActive_; }

    bool IsActive() const { return IsSelfActive() && (HasParent() ? GetParent()->IsActive() : true); }

    void SetID(uint32_t id) { actorID_ = id; }

    uint32_t GetID() const { return actorID_; }

    void SetParent(LoongActor* parent);

    void DetachFromParent();

    bool HasParent() const { return parent_ != nullptr; }

    bool IsAncestorOf(const LoongActor* descent) const
    {
        while (descent->HasParent()) {
            descent = descent->GetParent();
            if (descent == this) {
                return true;
            }
        }
        return false;
    }

    LoongActor* GetParent() const { return parent_; }

    LoongActor* GetRoot()
    {
        auto* root = this;
        while (root->HasParent()) {
            root = root->GetParent();
        }
        return root;
    }

    uint32_t GetParentID() const { return HasParent() ? GetParent()->GetID() : 0; }

    std::vector<LoongActor*>& GetChildren() { return children_; }

    const std::vector<LoongActor*>& GetChildren() const { return children_; }

    LoongActor* GetChildByName(const std::string& name) const;

    LoongActor* GetChildByNameRecursive(const std::string& name) const;

    LoongActor* GetChildById(uint32_t id) const;

    LoongActor* GetChildByIdRecursive(uint32_t id) const;

    LoongActor* GetChildByTag(const std::string& tag) const;

    LoongActor* GetChildByTagRecursive(const std::string& tag) const;

    void MarkAsDestroy();

    void OnStart();

    void OnEnable();

    void OnDisable();

    void OnDestroy();

    bool IsAlive() const { return !isDestroyed_; }

    void OnUpdate(const Foundation::LoongClock& clock);

    void OnFixedUpdate(const Foundation::LoongClock& clock);

    void OnLateUpdate(const Foundation::LoongClock& clock);

    // void OnCollisionEnter(CPhysics& otherObject);
    //
    // void OnCollisionStay(CPhysics& otherObject);
    //
    // void OnCollisionExit(CPhysics& otherObject);
    //
    // void OnTriggerEnter(CPhysics& otherObject);
    //
    // void OnTriggerStay(CPhysics& otherObject);
    //
    // void OnTriggerExit(CPhysics& otherObject);

    template <typename T, typename... Args>
    T* AddComponent(Args&&... args)
    {
        // Every actor should have only one component of a certain type to improve performance
        static_assert(std::is_base_of<LoongComponent, T>::value, "T should derive from CComponent");

        if (auto found = GetComponent<T>(); found != nullptr) {
            return found;
        } else {
            std::shared_ptr<T> comp = std::make_shared<T>(this, std::forward<Args>(args)...);
            components_.push_back(comp);
            AddComponentSignal_.emit(this, comp.get());
            if (IsActive()) {
                static_cast<LoongComponent&>(*comp).OnEnable();
                static_cast<LoongComponent&>(*comp).OnStart();
            }
            return comp.get();
        }
    }

    template <typename T>
    bool RemoveComponent()
    {
        static_assert(std::is_base_of<LoongComponent, T>::value, "T should derive from AComponent");

        std::shared_ptr<T> result(nullptr);

        for (auto it = components_.begin(); it != components_.end(); ++it) {
            result = std::dynamic_pointer_cast<T>(*it);
            if (result) {
                RemoveComponentSignal_.emit(this, result.get());
                components_.erase(it);
                return true;
            }
        }

        return false;
    }

    bool RemoveComponent(LoongComponent* component);

    template <typename T>
    T* GetComponent()
    {
        static_assert(std::is_base_of<LoongComponent, T>::value, "T should derive from AComponent");

        std::shared_ptr<T> result(nullptr);

        for (auto& component : components_) {
            result = std::dynamic_pointer_cast<T>(component);
            if (result) {
                return result.get();
            }
        }

        return nullptr;
    }

    const std::vector<std::shared_ptr<LoongComponent>>& GetComponents() { return components_; }

    // virtual void OnSerialize(tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* actorsRoot) override;

    // virtual void OnDeserialize(tinyxml2::XMLDocument& doc, tinyxml2::XMLNode* actorsRoot) override;

    LOONG_DECLARE_SIGNAL(Destroy, LoongActor*); // the first is this, the second is parent
    LOONG_DECLARE_SIGNAL(DetachFromParent, LoongActor*, LoongActor*); // the first is this, the second is parent
    LOONG_DECLARE_SIGNAL(AttachToParent, LoongActor*, LoongActor*); // the first is this, the second is parent
    LOONG_DECLARE_SIGNAL(RemoveComponent, LoongActor*, LoongComponent*); // the first is this, the second is component
    LOONG_DECLARE_SIGNAL(AddComponent, LoongActor*, LoongComponent*); // the first is this, the second is component

    const Foundation::Transform& GetTransform() const { return transform_; }

    Foundation::Transform& GetTransform() { return transform_; }

private:
    void RecursiveActiveUpdate();
    void RecursiveWasActiveUpdate();

private:
    std::string name_ {};
    std::string tag_ {};
    bool isActive_ { true };
    bool wasActive_ { false };
    bool isDestroyed_ { false };
    bool isStarted_ { false };

    uint32_t actorID_ { 0 };

    LoongActor* parent_ { nullptr };
    std::vector<LoongActor*> children_ {};

    std::vector<std::shared_ptr<LoongComponent>> components_ {};

    Foundation::Transform transform_ {};

    friend class LoongScene;
};

}