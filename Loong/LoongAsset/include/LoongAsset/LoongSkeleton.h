//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include "LoongFoundation/LoongMath.h"
#include <string>
#include <vector>

namespace Loong::Asset {

class LoongSkeleton {
public:
    struct Node {
        std::string name {};
        std::vector<Node> children {};
        Math::Matrix4 transform {};

        template <class Archive>
        bool Serialize(Archive& archive) { return archive(name, children, transform); }
    };

    LoongSkeleton() = default;

    // Note: this function will take over the ownship
    LoongSkeleton(Node&& root)
        : root_(std::move(root))
        , inverseMatrix_()
    {
        inverseMatrix_ = Math::Inverse(root_.transform);
    }

    const Math::Matrix4& GetInverseMatrix() const { return inverseMatrix_; }

    template <class Archive>
    bool Serialize(Archive& archive) { return archive(root_, inverseMatrix_); }

private:
    Node root_ { };
    // This is the inverse of the root node's transform, used to transform the skeleton to model space
    Math::Matrix4 inverseMatrix_ {};
};

}
