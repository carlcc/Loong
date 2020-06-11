//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Loong::Editor {

class LoongFileTreeNode {
public:
    explicit LoongFileTreeNode(std::string fileName, LoongFileTreeNode* parent = nullptr)
        : fileName(std::move(fileName))
        , parent(parent)
    {
    }

    std::string GetFullPath() const
    {
        if (parent == nullptr) {
            return "";
        } else {
            return parent->GetFullPath() + '/' + fileName;
        }
    }

    std::string fileName {};
    LoongFileTreeNode* parent { nullptr };
    std::vector<std::shared_ptr<LoongFileTreeNode>> children {};
    bool isDir { false };
    bool isScanned { false };
};

}