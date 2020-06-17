//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMath.h"
#include "LoongFoundation/LoongSigslotHelper.h"

namespace Loong::Foundation {

class Transform : public LoongHasSlots {
public:
    explicit Transform(Math::Vector3 pos = Math::Zero, Math::Quat rot = Math::Identity, Math::Vector3 scl = Math::One)
        : position_(pos)
        , rotation_(rot)
        , scale_(scl)
        , parent_(nullptr)
    {
    }

    void SetPosition(const Math::Vector3& pos)
    {
        position_ = pos;
        OnTransformChange();
    }

    void SetWorldPosition(const Math::Vector3& pos)
    {
        if (parent_ == nullptr) {
            position_ = pos;
        } else {
            // T_world = T_parent * T_local ====> T_local = T_parent^-1 * T_world
            position_ = Math::Inverse(parent_->GetWorldTransformMatrix()) * Math::Vector4 { pos, 1.0F };
        }
        OnTransformChange();
    }

    void SetRotation(const Math::Quat& rot)
    {
        rotation_ = rot;
        OnTransformChange();
    }

    void SetWorldRotation(const Math::Quat& rot)
    {
        if (parent_ == nullptr) {
            rotation_ = rot;
        } else {
            rotation_ = Math::Inverse(parent_->GetWorldRotation()) * rot;
        }
        OnTransformChange();
    }

    void SetScale(const Math::Vector3& scl)
    {
        scale_ = scl;
        OnTransformChange();
    }

    void Translate(const Math::Vector3& trans)
    {
        position_ += trans;
        OnTransformChange();
    }

    void Rotate(const Math::Quat& rot)
    {
        rotation_ = Math::Normalize(rotation_ * rot);
        OnTransformChange();
    }

    void Rotate(const Math::Vector3& axis, float radian)
    {
        rotation_ = Math::Rotate(rotation_, axis, radian);
        OnTransformChange();
    }

    void Scale(const Math::Vector3& scl)
    {
        scale_ *= scl;
        OnTransformChange();
    }

    void LookAt(const Math::Vector3& target, const Math::Vector3& up)
    {
        rotation_ = Math::Conjugate(Math::MatrixToQuat(Math::LookAt(position_, target, up)));
        OnTransformChange();
    }

    const Math::Vector3& GetPosition() const { return position_; }
    const Math::Vector3& GetWorldPosition() const
    {
        UpdateWorldCache();
        return worldPosition_;
    }

    const Math::Quat& GetRotation() const { return rotation_; }
    const Math::Quat& GetWorldRotation()
    {
        UpdateWorldCache();
        return worldRotation_;
    }

    const Math::Vector3& GetScale() const { return scale_; }
    const Math::Vector3& GetWorldScale() const
    {
        UpdateWorldCache();
        return worldScale_;
    }

    Math::Vector3 GetForward() const { return rotation_ * Math::kForward; }
    Math::Vector3 GetWorldForward() const
    {
        UpdateWorldCache();
        return worldRotation_ * Math::kForward;
    }

    Math::Vector3 GetUp() const { return rotation_ * Math::kUp; }
    Math::Vector3 GetWorldUp() const
    {
        UpdateWorldCache();
        return worldRotation_ * Math::kUp;
    }

    Math::Vector3 GetRight() const { return rotation_ * Math::kRight; }
    Math::Vector3 GetWorldRight() const
    {
        UpdateWorldCache();
        return worldRotation_ * Math::kRight;
    }

    const Math::Matrix4& GetTransformMatrix() const
    {
        UpdateLocalCache();
        return localMatrix_;
    }

    const Math::Matrix4& GetWorldTransformMatrix() const
    {
        UpdateWorldCache();
        return worldMatrix_;
    }

    Transform* GetParent() const { return parent_; }

    void SetParent(Transform* parent)
    {
        if (parent == parent_) {
            return;
        }
        if (parent_ != nullptr) {
            parent_->UnsubscribeTransformChange(this);
        }
        parent_ = parent;
        // TODO: Calculate a new transform to keep global transform not change?
        OnTransformChange();
        if (parent_ != nullptr) {
            parent_->SubscribeTransformChange(this, &Transform::OnTransformChange);
        }
    }

    LOONG_DECLARE_SIGNAL(TransformChange);

private:
    void OnTransformChange()
    {
        isWorldDirty_ = true;
        isLocalDirty_ = true;
        TransformChangeSignal_.emit();
    }

    void UpdateLocalCache() const
    {
        if (isLocalDirty_) {
            isLocalDirty_ = false;
            localMatrix_ = Math::Translate(position_) * Math::QuatToMatrix4(rotation_) * Math::Scale(scale_);
        }
    }

    void UpdateWorldCache() const
    {
        if (isWorldDirty_) {
            isWorldDirty_ = false;
            if (parent_ != nullptr) {
                worldMatrix_ = parent_->GetWorldTransformMatrix() * GetTransformMatrix();
                Math::Decompose(worldMatrix_, worldScale_, worldRotation_, worldPosition_);
            } else {
                worldPosition_ = position_;
                worldScale_ = scale_;
                worldRotation_ = rotation_;
                worldMatrix_ = GetTransformMatrix();
            }
        }
    }

private:
    Math::Vector3 position_ { Math::Zero };
    Math::Quat rotation_ { Math::Identity };
    Math::Vector3 scale_ { Math::One };
    Transform* parent_ { nullptr };

    mutable bool isLocalDirty_ { true };
    mutable bool isWorldDirty_ { true };
    mutable Math::Matrix4 localMatrix_ {};
    mutable Math::Matrix4 worldMatrix_ {};
    mutable Math::Vector3 worldPosition_ {};
    mutable Math::Quat worldRotation_ {};
    mutable Math::Vector3 worldScale_ {};
};

}
