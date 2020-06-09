//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <string>

namespace Loong::Foundation {
class LoongClock;
}

namespace Loong::Core {

class LoongActor;

class LoongComponent {
public:
    explicit LoongComponent(LoongActor* owner)
        : owner_(owner)
    {
    }

    LoongComponent(const LoongComponent&) = delete;
    LoongComponent(LoongComponent&&) = delete;

    virtual ~LoongComponent() = default;

    LoongComponent& operator=(const LoongComponent&) = delete;
    LoongComponent& operator=(LoongComponent&&) = delete;

    LoongActor* GetOwner() const { return owner_; }

    virtual void OnStart() {}

    virtual void OnEnable() {}

    virtual void OnDisable() {}

    virtual void OnDestroy() {}

    virtual void OnUpdate(const Foundation::LoongClock& clock) {}

    virtual void OnFixedUpdate(const Foundation::LoongClock& clock) {}

    virtual void OnLateUpdate(const Foundation::LoongClock& clock) {}

    // virtual void OnCollisionEnter(CPhysics& otherObject) {}
    //
    // virtual void OnCollisionStay(CPhysics& otherObject) {}
    //
    // virtual void OnCollisionExit(CPhysics& otherObject) {}
    //
    // virtual void OnTriggerEnter(CPhysics& otherObject) {}
    //
    // virtual void OnTriggerStay(CPhysics& otherObject) {}
    //
    // virtual void OnTriggerExit(CPhysics& otherObject) {}

    virtual std::string GetName() = 0;

private:
    LoongActor* owner_;
};

}